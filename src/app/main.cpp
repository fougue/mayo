/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application.h"
#include "../base/document_tree_node_properties_provider.h"
#include "../base/io_system.h"
#include "../base/messenger.h"
#include "../base/settings.h"
#include "../base/task_manager.h"
#include "../io_dxf/io_dxf.h"
#include "../io_gmio/io_gmio.h"
#include "../io_image/io_image.h"
#include "../io_occ/io_occ.h"
#include "../io_ply/io_ply_reader.h"
#include "../graphics/graphics_object_driver.h"
#include "../gui/gui_application.h"
#include "../gui/qtgui_utils.h"
#include "app_module.h"
#include "console.h"
#include "document_tree_node_properties_providers.h"
#include "filepath_conv.h"
#include "mainwindow.h"
#include "qsettings_storage.h"
#include "qstring_conv.h"
#include "theme.h"
#include "version.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_mesh.h"
#include "widget_model_tree_builder_xde.h"
#include "widget_occ_view.h"

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QVersionNumber>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtWidgets/QApplication>

#include <Message.hxx>

#include <fmt/format.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <unordered_map>

#ifdef Q_OS_WIN
#  include <windows.h> // For AttachConsole(), etc.
#endif

namespace Mayo {

// Declared in graphics/graphics_create_driver.cpp
void setFunctionCreateGraphicsDriver(std::function<Handle_Graphic3d_GraphicDriver()> fn);

class Main {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Main)
    Q_DECLARE_TR_FUNCTIONS(Mayo::Main)
};

namespace {

struct CommandLineArguments {
    QString themeName;
    FilePath filepathSettings;
    FilePath filepathLog;
    bool includeDebugLogs = true;
    std::vector<FilePath> listFilepathToExport;
    std::vector<FilePath> listFilepathToOpen;
    bool cliProgressReport = true;
};

class LogMessageHandler {
public:
    static LogMessageHandler& instance()
    {
        static LogMessageHandler object;
        return object;
    }

    void enableDebugLogs(bool on)
    {
        m_enableDebugLogs = on;
    }

    void setOutputFilePath(const FilePath& fp)
    {
        m_outputFilePath = fp;
        m_outputFile.open(fp, std::ios::out | std::ios::app);
    }

    std::ostream& outputStream(QtMsgType type)
    {
        if (!m_outputFilePath.empty() && m_outputFile.is_open())
            return m_outputFile;

        if (type == QtDebugMsg || type == QtInfoMsg)
            return std::cout;

        return std::cerr;
    }

    static void qtHandler(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
    {
        const std::string localMsg = consoleToPrintable(msg);
        std::ostream& outs = LogMessageHandler::instance().outputStream(type);
        switch (type) {
        case QtDebugMsg:
            if (LogMessageHandler::instance().m_enableDebugLogs) {
                outs << "DEBUG: " << localMsg << std::endl;
            }
            break;
        case QtInfoMsg:
            outs << "INFO: " << localMsg << std::endl;
            break;
        case QtWarningMsg:
            outs << "WARNING: " << localMsg << std::endl;
            break;
        case QtCriticalMsg:
            outs << "CRITICAL: " << localMsg << std::endl;
            break;
        case QtFatalMsg:
            outs << "FATAL: " << localMsg << std::endl;
            break;
        }
    }

private:
    LogMessageHandler() = default;

    FilePath m_outputFilePath;
    std::ofstream m_outputFile;
    bool m_enableDebugLogs = true;
};

} // namespace

static CommandLineArguments processCommandLine()
{
    CommandLineArguments args;

    // Configure command-line parser
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                Main::tr("Mayo, an open-source 3D viewer based on Qt5/OpenCascade"));
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();

    const QCommandLineOption cmdOptionTheme(
                QStringList{ "t", "theme" },
                Main::tr("Theme for the UI(classic|dark)"),
                Main::tr("name"));
    cmdParser.addOption(cmdOptionTheme);

    const QCommandLineOption cmdFileSettings(
                QStringList{ "s", "settings" },
                Main::tr("Settings file(INI format) to load at startup"),
                Main::tr("filepath"));
    cmdParser.addOption(cmdFileSettings);

    const QCommandLineOption cmdFileToExport(
                QStringList{ "e", "export" },
                Main::tr("Export opened files into an output file, can be repeated for different "
                         "formats(eg. -e file.stp -e file.igs...)"),
                Main::tr("filepath"));
    cmdParser.addOption(cmdFileToExport);

    const QCommandLineOption cmdFileLog(
                QStringList{ "log-file" },
                Main::tr("Writes log messages into output file"),
                Main::tr("filepath"));
    cmdParser.addOption(cmdFileLog);

    const QCommandLineOption cmdDebugLogs(
                QStringList{ "debug-logs" },
                Main::tr("Don't filter out debug log messages in release build"));
    cmdParser.addOption(cmdDebugLogs);

    const QCommandLineOption cmdCliNoProgress(
                QStringList{ "no-progress" },
                Main::tr("Disable progress reporting in console output(CLI-mode only)"));
    cmdParser.addOption(cmdCliNoProgress);

    cmdParser.addPositionalArgument(
                Main::tr("files"),
                Main::tr("Files to open at startup, optionally"),
                Main::tr("[files...]"));

    cmdParser.process(QCoreApplication::arguments());

    // Retrieve arguments
    args.themeName = "dark";
    if (cmdParser.isSet(cmdOptionTheme))
        args.themeName = cmdParser.value(cmdOptionTheme);

    if (cmdParser.isSet(cmdFileSettings))
        args.filepathSettings = filepathFrom(cmdParser.value(cmdFileSettings));

    if (cmdParser.isSet(cmdFileLog))
        args.filepathLog = filepathFrom(cmdParser.value(cmdFileLog));

    if (cmdParser.isSet(cmdFileToExport)) {
        for (const QString& strFilepath : cmdParser.values(cmdFileToExport))
            args.listFilepathToExport.push_back(filepathFrom(strFilepath));
    }

    for (const QString& posArg : cmdParser.positionalArguments())
        args.listFilepathToOpen.push_back(filepathFrom(posArg));

#ifdef NDEBUG
    // By default this will exclude debug logs in release build
    args.includeDebugLogs = cmdParser.isSet(cmdDebugLogs);
#endif
    args.cliProgressReport = !cmdParser.isSet(cmdCliNoProgress);

    return args;
}

static std::unique_ptr<Theme> globalTheme;

// Declared in theme.h
Theme* mayoTheme()
{
    return globalTheme.get();
}

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

// Initializes "Base" objects
static void initBase(QCoreApplication* qtApp)
{
    auto app = Application::instance();
    app->settings()->setStorage(std::make_unique<QSettingsStorage>());

    // Load translation files
    {
        const QString qmFilePath = AppModule::qmFilePath(AppModule::languageCode(app));
        auto translator = new QTranslator(app.get());
        if (translator->load(qmFilePath))
            qtApp->installTranslator(translator);
        else
            qWarning() << Main::tr("Failed to load translation file [path=%1]").arg(qmFilePath);
    }

    // Set Qt i18n backend
    app->addTranslator([=](const TextId& text, int n) -> std::string_view {
        const QString qstr = qtApp->translate(text.trContext.data(), text.key.data(), nullptr, n);
        auto qstrHash = qHash(qstr);
        static std::unordered_map<unsigned, std::string> mapStr;
        static QReadWriteLock mapStrLock;
        {
            QReadLocker locker(&mapStrLock);
            auto it = mapStr.find(qstrHash);
            if (it != mapStr.cend())
                return it->second;
        }

        QWriteLocker locker(&mapStrLock);
        auto [it, ok] = mapStr.insert({ qstrHash, to_stdString(qstr) });
        return ok ? it->second : std::string_view{};
    });

    // Register I/O objects
    app->ioSystem()->addFactoryReader(std::make_unique<IO::OccFactoryReader>());
    app->ioSystem()->addFactoryReader(std::make_unique<IO::DxfFactoryReader>());
    app->ioSystem()->addFactoryReader(std::make_unique<IO::PlyFactoryReader>());
    app->ioSystem()->addFactoryWriter(std::make_unique<IO::OccFactoryWriter>());
    app->ioSystem()->addFactoryWriter(IO::GmioFactoryWriter::create());
    IO::addPredefinedFormatProbes(app->ioSystem());

    // Register providers to query document tree node properties
    app->documentTreeNodePropertiesProviderTable()->addProvider(
                std::make_unique<XCaf_DocumentTreeNodePropertiesProvider>());
    app->documentTreeNodePropertiesProviderTable()->addProvider(
                std::make_unique<Mesh_DocumentTreeNodePropertiesProvider>());
}

// Helper to query the OpenGL version string
static std::string queryGlVersionString()
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
static QVersionNumber parseSemanticVersionString(std::string_view strVersion)
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
    if (qobject_cast<QGuiApplication*>(QCoreApplication::instance())) { // QOpenGL requires QGuiApplication
        const std::string strGlVersion = queryGlVersionString();
        const QVersionNumber glVersion = parseSemanticVersionString(strGlVersion);
        qInfo() << fmt::format("OpenGL v{}.{}", glVersion.majorVersion(), glVersion.minorVersion()).c_str();
        if (!glVersion.isNull() && glVersion.majorVersion() >= 2) { // Requires at least OpenGL version >= 2.0
            setFunctionCreateGraphicsDriver(&QOpenGLWidgetOccView::createCompatibleGraphicsDriver);
            IWidgetOccView::setCreator(&QOpenGLWidgetOccView::create);
        }
        else {
            qWarning() << "Can't use QOpenGLWidget because OpenGL version is too old";
        }
    }
#endif

    // Register I/O objects
    guiApp->application()->ioSystem()->addFactoryWriter(std::make_unique<IO::ImageFactoryWriter>(guiApp));

    // Register Graphics/TreeNode mapping drivers
    guiApp->graphicsTreeNodeMappingDriverTable()->addDriver(
                std::make_unique<GraphicsShapeTreeNodeMappingDriver>());

    // Register Graphics entity drivers
    guiApp->graphicsObjectDriverTable()->addDriver(std::make_unique<GraphicsShapeObjectDriver>());
    guiApp->graphicsObjectDriverTable()->addDriver(std::make_unique<GraphicsMeshObjectDriver>());
}

// Asynchronously exports input file(s) listed in 'args'
// Calls 'fnContinuation' at the end of execution
static void cli_asyncExportDocuments(
        Application* app, const CommandLineArguments& args, std::function<void(int)> fnContinuation)
{
    struct TaskStatus {
        std::atomic<bool> finished = {};
        std::atomic<bool> success = {};
    };

    struct Helper : public QObject {
        // Task manager object dedicated to the scope of current function
        TaskManager taskMgr;
        // Counter decremented for each export task finished, when 0 is reached then quit
        std::atomic<int> exportTaskCount = {};
        // Mapping between a task id and the task status
        std::unordered_map<TaskId, std::unique_ptr<TaskStatus>> mapTaskStatus;
        // Mapping between a task id and corresponding width of the progress line in console
        std::unordered_map<TaskId, int> mapTaskLineWidth;
        // Count of progress lines in console after last call to fnPrintProgress()
        int lastPrintProgressLineCount = 0;
    };

    // Collects emitted error messages into a single string object
    class ErrorMessageCollect : public Messenger {
    public:
        void emitMessage(MessageType msgType, std::string_view text) override {
            if (msgType == MessageType::Error)
                m_message += to_QString(text) + " ";
        }

        std::string message() const { return to_stdString(m_message); }

    private:
        QString m_message;
    };

    auto helper = new Helper; // Allocated on heap because current function is asynchronous
    auto taskMgr = &helper->taskMgr;
    auto appModule = AppModule::get(app);

    // Helper function to exit current function
    auto fnExit = [=](int retCode) {
        helper->deleteLater();
        fnContinuation(retCode);
    };
    // Helper function to print in console the progress info of current function
    auto fnPrintProgress = [=]{
        consoleCursorMoveUp(helper->lastPrintProgressLineCount);
        helper->lastPrintProgressLineCount = 0;
        std::cout << "\r";
        taskMgr->foreachTask([=](TaskId taskId) {
            std::string strMessage = taskMgr->title(taskId);
            std::replace(strMessage.begin(), strMessage.end(), '\n', ' ');
            strMessage = consoleToPrintable(strMessage);
            auto lineWidth = int(strMessage.size());
            const bool taskFinished = helper->mapTaskStatus.at(taskId)->finished;
            const bool taskSuccess = helper->mapTaskStatus.at(taskId)->success;
            if (taskFinished && !taskSuccess) {
                consoleSetTextColor(ConsoleColor::Red);
                std::cout << strMessage;
                consoleSetTextColor(ConsoleColor::Default);
            }
            else {
                const int progress = taskMgr->progress(taskId);
                if (progress >= 100)
                    consoleSetTextColor(ConsoleColor::Green);

                std::cout << std::setfill(' ') << std::right << std::setw(3) << progress << "% ";
                std::cout << strMessage;
                lineWidth += 5;
                if (progress >= 100)
                    consoleSetTextColor(ConsoleColor::Default);
            }

            const int printWidth = consoleWidth();
            auto itLineFound = helper->mapTaskLineWidth.find(taskId);
            const int lineWidthOld = itLineFound != helper->mapTaskLineWidth.cend() ? itLineFound->second : printWidth - 1;
            for (int i = 0; i < (lineWidthOld - lineWidth); ++i)
                std::cout << ' ';

            helper->mapTaskLineWidth.insert_or_assign(taskId, lineWidth);
            helper->lastPrintProgressLineCount += printWidth > 0 ? (lineWidth / printWidth) + 1 : 1;
            std::cout << "\n";
        });
        std::cout.flush();
    };

    // Show progress/traces corresponding to task events
    QObject::connect(taskMgr, &TaskManager::started, app, [=](TaskId taskId) {
        if (args.cliProgressReport)
            fnPrintProgress();
        else
            qInfo() << to_QString(taskMgr->title(taskId));
    });
    QObject::connect(taskMgr, &TaskManager::ended, app, [=](TaskId taskId) {
        if (args.cliProgressReport) {
            fnPrintProgress();
        }
        else {
            if (helper->mapTaskStatus.at(taskId)->success)
                qInfo() << to_QString(taskMgr->title(taskId));
            else
                qCritical() << to_QString(taskMgr->title(taskId));
        }
    });
    QObject::connect(taskMgr, &TaskManager::progressChanged, app, [=]{
        if (args.cliProgressReport)
            fnPrintProgress();
    });

    helper->exportTaskCount = int(args.listFilepathToExport.size());
    QObject::connect(taskMgr, &TaskManager::ended, app, [=]{
        if (helper->exportTaskCount == 0) {
            bool okExport = true;
            for (const auto& mapPair : helper->mapTaskStatus) {
                const TaskStatus* status = mapPair.second.get();
                okExport = okExport && status->success;
            }

            fnExit(okExport ? EXIT_SUCCESS : EXIT_FAILURE);
        }
    });

    // If export operation targets some mesh format then force meshing of imported BRep shapes
    bool brepMeshRequired = false;
    for (const FilePath& filepath : args.listFilepathToExport) {
        const IO::Format format = app->ioSystem()->probeFormat(filepath);
        brepMeshRequired = IO::formatProvidesMesh(format);
        if (brepMeshRequired)
            break; // Interrupt
    }

    // Suppress output from OpenCascade
    Message::DefaultMessenger()->RemovePrinters(Message_Printer::get_type_descriptor());

    // Execute import operation(synchronous)
    DocumentPtr doc = app->newDocument();
    bool okImport = true;
    const TaskId importTaskId = taskMgr->newTask([&](TaskProgress* progress) {
            ErrorMessageCollect errorCollect;
            okImport = app->ioSystem()->importInDocument()
                .targetDocument(doc)
                .withFilepaths(args.listFilepathToOpen)
                .withParametersProvider(appModule)
                .withEntityPostProcess([=](TDF_Label labelEntity, TaskProgress* progress) {
                    appModule->computeBRepMesh(labelEntity, progress);
                })
                .withEntityPostProcessRequiredIf([=](IO::Format){ return brepMeshRequired; })
                .withEntityPostProcessInfoProgress(20, Main::textIdTr("Mesh BRep shapes"))
                .withMessenger(&errorCollect)
                .withTaskProgress(progress)
                .execute();
            taskMgr->setTitle(progress->taskId(), okImport ? Main::textIdTr("Imported") : errorCollect.message());
            helper->mapTaskStatus.at(progress->taskId())->success = okImport;
            helper->mapTaskStatus.at(progress->taskId())->finished = true;
    });
    helper->mapTaskStatus.insert({ importTaskId, std::make_unique<TaskStatus>() });
    taskMgr->setTitle(importTaskId, Main::textIdTr("Importing..."));
    taskMgr->exec(importTaskId, TaskAutoDestroy::Off);
    if (!okImport)
        return fnExit(EXIT_FAILURE); // Error

    // Run export operations(asynchronous)
    for (const FilePath& filepath : args.listFilepathToExport) {
        const std::string strFilename = filepath.filename().u8string();
        const TaskId taskId = taskMgr->newTask([=](TaskProgress* progress) {
                ErrorMessageCollect errorCollect;
                const IO::Format format = app->ioSystem()->probeFormat(filepath);
                const ApplicationItem appItems[] = { doc };
                const bool okExport = app->ioSystem()->exportApplicationItems()
                            .targetFile(filepath)
                            .targetFormat(format)
                            .withItems(appItems)
                            .withParameters(appModule->findWriterParameters(format))
                            .withMessenger(&errorCollect)
                            .withTaskProgress(progress)
                            .execute();
                const std::string msg =
                        okExport ?
                            fmt::format(Main::textIdTr("Exported {}"), strFilename) :
                            errorCollect.message();
                taskMgr->setTitle(progress->taskId(), msg);
                helper->mapTaskStatus.at(progress->taskId())->success = okExport;
                helper->mapTaskStatus.at(progress->taskId())->finished = true;
                --(helper->exportTaskCount);
        });
        helper->mapTaskStatus.insert({ taskId, std::make_unique<TaskStatus>() });
        taskMgr->setTitle(taskId, fmt::format(Main::textIdTr("Exporting {}..."), strFilename));
    }

    taskMgr->foreachTask([=](TaskId taskId) {
        if (taskId != importTaskId)
            taskMgr->run(taskId, TaskAutoDestroy::Off);
    });
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

    // Message logging
    LogMessageHandler::instance().enableDebugLogs(args.includeDebugLogs);
    if (!args.filepathLog.empty())
        LogMessageHandler::instance().setOutputFilePath(args.filepathLog);

    // Initialize Base application
    initOpenCascadeEnvironment("opencascade.conf");
    initBase(qtApp);
    auto app = Application::instance().get();

    // Initialize Gui application
    auto guiApp = new GuiApplication(app);
    initGui(guiApp);

    // Register AppModule
    auto appModule = new AppModule(app);
    app->settings()->setPropertyValueConversion(*appModule);

    // Process CLI
    if (!args.listFilepathToExport.empty()) {
        if (args.listFilepathToOpen.empty())
            fnCriticalExit(Main::tr("No input files -> nothing to export"));

        guiApp->setAutomaticDocumentMapping(false); // GuiDocument objects aren't needed
        app->settings()->resetAll();
        fnLoadAppSettings(app->settings());
        QTimer::singleShot(0, qtApp, [=]{
            cli_asyncExportDocuments(app, args, [=](int retcode) { qtApp->exit(retcode); });
        });
        return qtApp->exec();
    }

    // Record recent files when documents are closed
    QObject::connect(
                guiApp, &GuiApplication::guiDocumentErased,
                appModule, &AppModule::recordRecentFileThumbnail);

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
    mainWindow.show();
    if (!args.listFilepathToOpen.empty()) {
        QTimer::singleShot(0, [&]{ mainWindow.openDocumentsFromList(args.listFilepathToOpen); });
    }

    app->settings()->resetAll();
    fnLoadAppSettings(app->settings());
    const int code = qtApp->exec();
    appModule->recordRecentFileThumbnails(guiApp);
    app->settings()->save();
    return code;
}

static bool isAppCliMode = false;
static void onQtAppExit()
{
#if defined(Q_OS_WIN) && defined(NDEBUG)
    if (isAppCliMode)
        consoleSendEnterKey();
#endif
}

} // namespace Mayo

int main(int argc, char* argv[])
{
    qInstallMessageHandler(&Mayo::LogMessageHandler::qtHandler);
    qAddPostRoutine(&Mayo::onQtAppExit);

    // Running CLI mode?
    for (int i = 1; i < argc && !Mayo::isAppCliMode; ++i) {
        static const char* cliArgs[] = { "-e", "--export", "-h", "--help", "-v", "--version" };
        auto itCliArg = std::find_if(std::cbegin(cliArgs), std::cend(cliArgs), [=](const char* cliArg) {
            return std::strcmp(argv[i], cliArg) == 0;
        });
        Mayo::isAppCliMode = itCliArg != std::cend(cliArgs);
    }

#if defined(Q_OS_WIN)
    // Never use ANGLE on Windows, since OCCT 3D Viewer does not expect this
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    std::unique_ptr<QCoreApplication> ptrApp(
            Mayo::isAppCliMode ? new QCoreApplication(argc, argv) : new QApplication(argc, argv));

#if defined(Q_OS_WIN) && defined(NDEBUG)
    if (Mayo::isAppCliMode) {
        // https://devblogs.microsoft.com/oldnewthing/20090101-00/?p=19643
        // https://www.tillett.info/2013/05/13/how-to-create-a-windows-program-that-works-as-both-as-a-gui-and-console-application/
        if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
            auto fnRedirectToConsole = [](DWORD hnd, FILE* file, const char* strPath) {
                if (GetStdHandle(hnd) != INVALID_HANDLE_VALUE) {
                    freopen(strPath, "w", file);
                    setvbuf(file, nullptr, _IONBF, 0);
                }
            };
            fnRedirectToConsole(STD_OUTPUT_HANDLE, stdout, "CONOUT$");
            fnRedirectToConsole(STD_ERROR_HANDLE, stderr, "CONOUT$");
            std::ios::sync_with_stdio();
        }
    }
#endif

    QCoreApplication::setOrganizationName("Fougue Ltd");
    QCoreApplication::setOrganizationDomain("www.fougue.pro");
    QCoreApplication::setApplicationName("Mayo");
    QCoreApplication::setApplicationVersion(QString::fromUtf8(Mayo::strVersion));
    return Mayo::runApp(ptrApp.get());
}
