/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../io_assimp/io_assimp.h"
#include "../io_dxf/io_dxf.h"
#include "../io_gmio/io_gmio.h"
#include "../io_image/io_image.h"
#include "../io_occ/io_occ.h"
#include "../io_off/io_off_reader.h"
#include "../io_off/io_off_writer.h"
#include "../io_ply/io_ply_reader.h"
#include "../io_ply/io_ply_writer.h"
#include "../graphics/graphics_mesh_object_driver.h"
#include "../graphics/graphics_point_cloud_object_driver.h"
#include "../graphics/graphics_shape_object_driver.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_application.h"
#include "../qtbackend/qt_app_translator.h"
#include "../qtbackend/qt_signal_thread_helper.h"
#include "../qtbackend/qsettings_storage.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/log_message_handler.h"
#include "app_module.h"
#include "commands_help.h"
#include "document_tree_node_properties_providers.h"
#include "mainwindow.h"
#include "qtgui_utils.h"
#include "theme.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_mesh.h"
#include "widget_model_tree_builder_xde.h"
#include "widget_occ_view.h"
#include <common/mayo_version.h>

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QVersionNumber>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QPixmap>
#include <QtWidgets/QApplication>

#include <fmt/format.h>
#include <gsl/util>
#include <cstdlib>
#include <memory>

namespace Mayo {

// Declared in graphics/graphics_create_driver.cpp
void setFunctionCreateGraphicsDriver(std::function<OccHandle<Graphic3d_GraphicDriver>()> fn);

// Provides an i18n context for the current file(main.cpp)
class Main {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Main)
    Q_DECLARE_TR_FUNCTIONS(Mayo::Main)
};

namespace {

// Stores arguments(options) passed at command line
struct CommandLineArguments {
    QString themeName;
    FilePath filepathSettings;
    FilePath filepathLog;
    bool includeDebugLogs = true;
    std::vector<FilePath> listFilepathToOpen;
    bool showSystemInformation = false;
};

} // namespace

// Parses command line and process Qt builtin options(basically --version and --help)
static CommandLineArguments processCommandLine()
{
    CommandLineArguments args;

    // Configure command-line parser
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                Main::tr("Mayo the opensource 3D CAD viewer and converter")
    );
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    const QCommandLineOption cmdOptionTheme(
                QStringList{ "t", "theme" },
                Main::tr("Theme for the UI(classic|dark)"),
                Main::tr("name")
    );
    cmdParser.addOption(cmdOptionTheme);

    const QCommandLineOption cmdFileSettings(
                QStringList{ "s", "settings" },
                Main::tr("Settings file(INI format) to load at startup"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdFileSettings);

    const QCommandLineOption cmdFileLog(
                QStringList{ "log-file" },
                Main::tr("Writes log messages into output file"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdFileLog);

    const QCommandLineOption cmdDebugLogs(
                QStringList{ "debug-logs" },
                Main::tr("Don't filter out debug log messages in release build")
    );
    cmdParser.addOption(cmdDebugLogs);

    const QCommandLineOption cmdSysInfo(
                QStringList{ "system-info" },
                Main::tr("Show detailed system information and quit")
    );
    cmdParser.addOption(cmdSysInfo);

    cmdParser.addPositionalArgument(
                Main::tr("files"),
                Main::tr("Files to open at startup, optionally"),
                Main::tr("[files...]")
    );

    cmdParser.process(QCoreApplication::arguments());

    // Retrieve arguments
    args.themeName = "dark";
    if (cmdParser.isSet(cmdOptionTheme))
        args.themeName = cmdParser.value(cmdOptionTheme);

    if (cmdParser.isSet(cmdFileSettings))
        args.filepathSettings = filepathFrom(cmdParser.value(cmdFileSettings));

    if (cmdParser.isSet(cmdFileLog))
        args.filepathLog = filepathFrom(cmdParser.value(cmdFileLog));

    for (const QString& posArg : cmdParser.positionalArguments())
        args.listFilepathToOpen.push_back(filepathFrom(posArg));

#ifdef NDEBUG
    // By default this will exclude debug logs in release build
    args.includeDebugLogs = cmdParser.isSet(cmdDebugLogs);
#endif
    args.showSystemInformation = cmdParser.isSet(cmdSysInfo);

    return args;
}

static std::unique_ptr<Theme> globalTheme;

// Declared in theme.h
Theme* mayoTheme()
{
    return globalTheme.get();
}

// Set OpenCascade environment variables defined in a settings file(INI format)
static void initOpenCascadeEnvironment(const FilePath& settingsFilepath)
{
    const QString strSettingsFilepath = filepathTo<QString>(settingsFilepath);
    if (!filepathExists(settingsFilepath) /* TODO Check readable */) {
        qDebug().noquote() << Main::tr("OpenCascade settings file doesn't exist or is not readable [path=%1]")
                              .arg(strSettingsFilepath);
        return;
    }

    const QSettings occSettings(strSettingsFilepath, QSettings::IniFormat);
    if (occSettings.status() != QSettings::NoError) {
        qDebug().noquote() << Main::tr("OpenCascade settings file could not be loaded with QSettings [path=%1]")
                              .arg(strSettingsFilepath);
        return;
    }

    // Process options
    for (const char* varName : Application::envOpenCascadeOptions()) {
        const QLatin1String qVarName(varName);
        if (occSettings.contains(qVarName)) {
            const QString strValue = occSettings.value(qVarName).toString();
            qputenv(varName, strValue.toUtf8());
            qDebug().noquote() << QString("%1 = %2").arg(qVarName).arg(strValue);
        }
    }

    // Process paths
    for (const char* varName : Application::envOpenCascadePaths()) {
        const QLatin1String qVarName(varName);
        if (occSettings.contains(qVarName)) {
            QString strPath = occSettings.value(qVarName).toString();
            if (QFileInfo(strPath).isRelative())
                strPath = QCoreApplication::applicationDirPath() + QDir::separator() + strPath;

            strPath = QDir::toNativeSeparators(strPath);
            qputenv(varName, strPath.toUtf8());
            qDebug().noquote() << QString("%1 = %2").arg(qVarName).arg(strPath);
        }
    }
}

// Helper to query the OpenGL version string
[[maybe_unused]] static std::string queryGlVersionString()
{
    QOpenGLContext glContext;
    if (!glContext.create())
        return {};

    QOffscreenSurface surface;
    surface.create();
    if (!glContext.makeCurrent(&surface))
        return {};

    auto glVersion = glContext.functions()->glGetString(GL_VERSION);
    if (!glVersion)
        return {};

    return reinterpret_cast<const char*>(glVersion);
}

// Helper to parse a string containing a semantic version eg "4.6.5 CodeNamed"
// Note: only major and minor versions are detected
[[maybe_unused]] static QVersionNumber parseSemanticVersionString(std::string_view strVersion)
{
    if (strVersion.empty())
        return {};

    const char* ptrVersionStart = strVersion.data();
    const char* ptrVersionEnd = ptrVersionStart + strVersion.size();
    const int versionMajor = std::atoi(ptrVersionStart);
    int versionMinor = 0;
    auto ptrDot = std::find(ptrVersionStart, ptrVersionEnd, '.');
    if (ptrDot != ptrVersionEnd)
        versionMinor = std::atoi(ptrDot + 1);

    return QVersionNumber(versionMajor, versionMinor);
}

Thumbnail createGuiDocumentThumbnail(GuiDocument* guiDoc, QSize size)
{
    Thumbnail thumbnail;

    IO::ImageWriter::Parameters params;
    params.width = size.width();
    params.height = size.height();
    params.backgroundColorStart = QtGuiUtils::toPreferredColorSpace(mayoTheme()->color(Theme::Color::Palette_Window));
    params.backgroundColorEnd = params.backgroundColorStart;
    OccHandle<Image_AlienPixMap> pixmap = IO::ImageWriter::createImage(guiDoc, params);
    if (!pixmap) {
        qDebug() << "Empty pixmap returned by IO::ImageWriter::createImage()";
        return thumbnail;
    }

    GraphicsUtils::ImagePixmap_flipY(*pixmap);
    Image_PixMap::SwapRgbaBgra(*pixmap);
    const QPixmap qPixmap = QtGuiUtils::toQPixmap(*pixmap);
    thumbnail.imageData = QtGuiUtils::toQByteArray(qPixmap);
    thumbnail.imageCacheKey = qPixmap.cacheKey();
    return thumbnail;
}

// Initializes "GUI" objects
static void initGui(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

    // Fallback for OpenGL
    setFunctionCreateGraphicsDriver(&QWidgetOccView::createCompatibleGraphicsDriver);
    IWidgetOccView::setCreator(&QWidgetOccView::create);

    // Use QOpenGLWidget if possible
#if OCC_VERSION_HEX >= 0x070600
    const auto& propForceOpenGlFallbackWidget = AppModule::get()->properties()->forceOpenGlFallbackWidget;
    AppModule::get()->settings()->loadProperty(&propForceOpenGlFallbackWidget);
    const bool hasQGuiApplication = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (!propForceOpenGlFallbackWidget && hasQGuiApplication) { // QOpenGL requires QGuiApplication
        const std::string strGlVersion = queryGlVersionString();
        const QVersionNumber glVersion = parseSemanticVersionString(strGlVersion);
        if (!glVersion.isNull() && glVersion.majorVersion() >= 2) { // Requires at least OpenGL version >= 2.0
            setFunctionCreateGraphicsDriver(&QOpenGLWidgetOccView::createCompatibleGraphicsDriver);
            IWidgetOccView::setCreator(&QOpenGLWidgetOccView::create);
        }
        else {
            qWarning() << "Can't use QOpenGLWidget because OpenGL version is too old";
        }
    }
#endif

    // Register Graphics entity drivers
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsShapeObjectDriver>());
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsMeshObjectDriver>());
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsPointCloudObjectDriver>());
}

// Initializes and runs Mayo application
static int runApp(QCoreApplication* qtApp)
{
    const CommandLineArguments args = processCommandLine();

    // Helper function: print critical message and exit application with code failure
    auto fnCriticalExit = [](const QString& msg) {
        qCritical().noquote() << msg;
        std::exit(EXIT_FAILURE);
    };

    // Helper function: load application settings from INI file(if provided) otherwise use the
    // application regular storage(eg registry on Windows)
    auto fnLoadAppSettings = [&](Settings* appSettings) {
        if (args.filepathSettings.empty()) {
            appSettings->load();
        }
        else {
            const QString strFilepathSettings = filepathTo<QString>(args.filepathSettings);
            if (!filepathIsRegularFile(args.filepathSettings))
                fnCriticalExit(Main::tr("Failed to load application settings file [path=%1]").arg(strFilepathSettings));

            QSettingsStorage fileSettings(strFilepathSettings, QSettings::IniFormat);
            appSettings->loadFrom(fileSettings, &AppModule::excludeSettingPredicate);
        }
    };

    // Signals
    setGlobalSignalThreadHelper(std::make_unique<QtSignalThreadHelper>());

    // Message logging
    LogMessageHandler::instance().enableDebugLogs(args.includeDebugLogs);
    LogMessageHandler::instance().setOutputFilePath(args.filepathLog);

    // Initialize AppModule
    auto appModule = AppModule::get();
    appModule->settings()->setStorage(std::make_unique<QSettingsStorage>());
    appModule->setRecentFileThumbnailRecorder(&createGuiDocumentThumbnail);
    appModule->addLibraryInfo(
        IO::AssimpLib::strName(), IO::AssimpLib::strVersion(), IO::AssimpLib::strVersionDetails()
    );
    appModule->addLibraryInfo(
        IO::GmioLib::strName(), IO::GmioLib::strVersion(), IO::GmioLib::strVersionDetails()
    );

    {
        // Load translation files
        auto fnLoadQmFile = [=](const QString& qmFilePath) {
            auto translator = new QTranslator(qtApp);
            if (translator->load(qmFilePath))
                qtApp->installTranslator(translator);
            else
                qWarning() << Main::tr("Failed to load translation file [path=%1]").arg(qmFilePath);
        };
        const QString appLangCode = appModule->languageCode();
        fnLoadQmFile(QString(":/i18n/mayo_%1.qm").arg(appLangCode));
        fnLoadQmFile(QString(":/i18n/qtbase_%1.qm").arg(appLangCode));
    }

    // Initialize Base application
    auto app = appModule->application();
    TextId::addTranslatorFunction(&qtAppTranslate); // Set Qt i18n backend
#ifdef MAYO_OS_WINDOWS
    initOpenCascadeEnvironment("opencascade.conf");
#endif

    // Initialize Gui application
    auto guiApp = new GuiApplication(app);
    auto _ = gsl::finally([=]{ delete guiApp; });
    initGui(guiApp);

    // Register providers to query document tree node properties
    appModule->addPropertiesProvider(std::make_unique<XCaf_DocumentTreeNodePropertiesProvider>());
    appModule->addPropertiesProvider(std::make_unique<Mesh_DocumentTreeNodePropertiesProvider>());
    appModule->addPropertiesProvider(std::make_unique<PointCloud_DocumentTreeNodePropertiesProvider>());

    // Register I/O objects
    IO::System* ioSystem = appModule->ioSystem();
    ioSystem->addFactoryReader(std::make_unique<IO::DxfFactoryReader>());
    ioSystem->addFactoryReader(std::make_unique<IO::OccFactoryReader>());
    ioSystem->addFactoryReader(std::make_unique<IO::OffFactoryReader>());
    ioSystem->addFactoryReader(std::make_unique<IO::PlyFactoryReader>());
    ioSystem->addFactoryReader(IO::AssimpFactoryReader::create());
    ioSystem->addFactoryWriter(std::make_unique<IO::OccFactoryWriter>());
    ioSystem->addFactoryWriter(std::make_unique<IO::OffFactoryWriter>());
    ioSystem->addFactoryWriter(std::make_unique<IO::PlyFactoryWriter>());
    ioSystem->addFactoryWriter(IO::GmioFactoryWriter::create());
    ioSystem->addFactoryWriter(std::make_unique<IO::ImageFactoryWriter>(guiApp));
    IO::addPredefinedFormatProbes(ioSystem);
    appModule->properties()->IO_bindParameters(ioSystem);
    appModule->properties()->retranslate();

    // Process CLI
    if (args.showSystemInformation) {
        CommandSystemInformation cmdSysInfo(nullptr);
        cmdSysInfo.execute();
        return qtApp->exec();
    }

    // Record recent files when documents are closed
    guiApp->signalGuiDocumentErased.connectSlot(&AppModule::recordRecentFile, AppModule::get());

    // Register WidgetModelTreeBuilter prototypes
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Mesh>());
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Xde>());

    // Create theme
    globalTheme.reset(createTheme(args.themeName));
    if (!globalTheme)
        fnCriticalExit(Main::tr("Failed to load theme '%1'").arg(args.themeName));

    mayoTheme()->setup();
    const QColor bkgGradientStart = mayoTheme()->color(Theme::Color::View3d_BackgroundGradientStart);
    const QColor bkgGradientEnd = mayoTheme()->color(Theme::Color::View3d_BackgroundGradientEnd);
    GuiDocument::setDefaultGradientBackground({
        QtGuiUtils::toPreferredColorSpace(bkgGradientStart),
        QtGuiUtils::toPreferredColorSpace(bkgGradientEnd),
        Aspect_GFM_VER
    });

    // Create MainWindow
    MainWindow mainWindow(guiApp);
    mainWindow.setWindowTitle(QCoreApplication::applicationName());
    appModule->settings()->loadProperty(&appModule->properties()->appUiState);
    mainWindow.show();
    if (!args.listFilepathToOpen.empty()) {
        QTimer::singleShot(0, qtApp, [&]{ mainWindow.openDocumentsFromList(args.listFilepathToOpen); });
    }

    appModule->settings()->resetAll();
    fnLoadAppSettings(appModule->settings());
    const int code = qtApp->exec();
    appModule->recordRecentFiles(guiApp);
    appModule->settings()->save();
    return code;
}

} // namespace Mayo

int main(int argc, char* argv[])
{
    qInstallMessageHandler(&Mayo::LogMessageHandler::qtHandler);

    // Helper function to check if application arguments contain any option listed in 'listOption'
    // IMPORTANT: capture by reference, because QApplication constructor may alter argc(due to
    //            parsing of arguments)
    auto fnArgsContainAnyOf = [&](std::initializer_list<const char*> listOption) {
        for (int i = 1; i < argc; ++i) {
            for (const char* option : listOption) {
                if (std::strcmp(argv[i], option) == 0)
                    return true;
            }
        }
        return false;
    };

    // OpenCascade TKOpenGl depends on XLib for Linux(excepting Android) and BSD systems(excepting macOS)
    // See for example implementation of Aspect_DisplayConnection where XLib is explicitly used
    // On systems running eg Wayland this would cause problems(see https://github.com/fougue/mayo/issues/178)
    // As a workaround the Qt platform is forced to xcb
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || (defined(Q_OS_BSD4) && !defined(Q_OS_MACOS))
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && !fnArgsContainAnyOf({ "-platform" }))
        qputenv("QT_QPA_PLATFORM", "xcb");
#elif defined(Q_OS_HAIKU)
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && !fnArgsContainAnyOf({ "-platform" }))
        qputenv("QT_QPA_PLATFORM", "haiku");
#endif

    // Configure and create Qt application object
#if defined(Q_OS_WIN)
    // Never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif
    QCoreApplication::setOrganizationName("Fougue Ltd");
    QCoreApplication::setOrganizationDomain("www.fougue.pro");
    QCoreApplication::setApplicationName("Mayo");
    QCoreApplication::setApplicationVersion(QString::fromUtf8(Mayo::strVersion));
    QApplication app(argc, argv);

    // Run Mayo application GUI
    return Mayo::runApp(&app);
}
