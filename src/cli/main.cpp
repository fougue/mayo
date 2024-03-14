/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../app/app_module.h"
#include "../base/application.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../graphics/graphics_mesh_object_driver.h"
#include "../graphics/graphics_point_cloud_object_driver.h"
#include "../graphics/graphics_shape_object_driver.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_application.h"
#include "../io_assimp/io_assimp.h"
#include "../io_dxf/io_dxf.h"
#include "../io_gmio/io_gmio.h"
#include "../io_image/io_image.h"
#include "../io_occ/io_occ.h"
#include "../io_off/io_off_reader.h"
#include "../io_off/io_off_writer.h"
#include "../io_ply/io_ply_reader.h"
#include "../io_ply/io_ply_writer.h"
#include "../qtbackend/qsettings_storage.h"
#include "../qtbackend/qt_app_translator.h"
#include "../qtbackend/qt_signal_thread_helper.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/log_message_handler.h"
#include "../qtcommon/qstring_conv.h"
#include "cli_export.h"
#include "console.h"
#include <common/mayo_version.h>

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>

#include <OpenGl_GraphicDriver.hxx>

#include <fmt/format.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

namespace Mayo {

// Declared in graphics/graphics_create_driver.cpp
void setFunctionCreateGraphicsDriver(std::function<Handle_Graphic3d_GraphicDriver()> fn);

// Provides an i18n context for the current file(main.cpp)
class Main {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Main)
    Q_DECLARE_TR_FUNCTIONS(Mayo::Main)
};

namespace {

// Stores arguments(options) passed at command line
struct CommandLineArguments {
    FilePath filepathSettings;
    FilePath filepathLog;
    bool includeDebugLogs = true;
    std::vector<FilePath> listFilepathToExport;
    std::vector<FilePath> listFilepathToOpen;
    bool cliProgressReport = true;
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

    const QCommandLineOption cmdFileSettings(
                QStringList{ "s", "settings" },
                Main::tr("Settings file(INI format) to load at startup"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdFileSettings);

    const QCommandLineOption cmdFileToExport(
                QStringList{ "e", "export" },
                Main::tr("Export opened files into an output file, can be repeated for different "
                         "formats(eg. -e file.stp -e file.igs...)"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdFileToExport);

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

    const QCommandLineOption cmdCliNoProgress(
                QStringList{ "no-progress" },
                Main::tr("Disable progress reporting in console output(CLI-mode only)")
    );
    cmdParser.addOption(cmdCliNoProgress);

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
    args.showSystemInformation = cmdParser.isSet(cmdSysInfo);

    return args;
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

// Initializes "GUI" objects
static void initGui(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

    setFunctionCreateGraphicsDriver([]() -> Handle_Graphic3d_GraphicDriver {
        return new OpenGl_GraphicDriver(GraphicsUtils::AspectDisplayConnection_create());
    });
    guiApp->addGraphicsObjectDriver(std::make_unique<GraphicsShapeObjectDriver>());
    guiApp->addGraphicsObjectDriver(std::make_unique<GraphicsMeshObjectDriver>());
    guiApp->addGraphicsObjectDriver(std::make_unique<GraphicsPointCloudObjectDriver>());
}

bool cliExcludingSettingPredicate(const Property& prop)
{
    return AppModule::excludeSettingPredicate(prop)
           || prop.dynTypeName() == PropertyRecentFiles::TypeName;
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
            appSettings->loadFrom(fileSettings, &cliExcludingSettingPredicate);
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
    auto app = Application::instance().get();
    app->addTranslator(&qtAppTranslate); // Set Qt i18n backend
    initOpenCascadeEnvironment("opencascade.conf");

    // Initialize Gui application
    auto guiApp = new GuiApplication(app);
    initGui(guiApp);

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

#if 0
    // Register library infos
    CommandSystemInformation::addLibraryInfo(
        IO::AssimpLib::strName(), IO::AssimpLib::strVersion(), IO::AssimpLib::strVersionDetails()
    );
    CommandSystemInformation::addLibraryInfo(
        IO::GmioLib::strName(), IO::GmioLib::strVersion(), IO::GmioLib::strVersionDetails()
    );

    // Process CLI
    if (args.showSystemInformation) {
        CommandSystemInformation cmdSysInfo(nullptr);
        cmdSysInfo.execute();
        return qtApp->exec();
    }
#endif

    if (!args.listFilepathToExport.empty()) {
        if (args.listFilepathToOpen.empty())
            fnCriticalExit(Main::tr("No input files -> nothing to export"));

        guiApp->setAutomaticDocumentMapping(false); // GuiDocument objects aren't needed
        appModule->settings()->resetAll();
        fnLoadAppSettings(appModule->settings());
        QTimer::singleShot(0, qtApp, [=]{
            CliExportArgs cliArgs;
            cliArgs.progressReport = args.cliProgressReport;
            cliArgs.filesToOpen = args.listFilepathToOpen;
            cliArgs.filesToExport = args.listFilepathToExport;
            cli_asyncExportDocuments(app, cliArgs, [=](int retcode) { qtApp->exit(retcode); });
        });
        return qtApp->exec();
    }

    return 0;
}

} // namespace Mayo

int main(int argc, char* argv[])
{
    qInstallMessageHandler(&Mayo::LogMessageHandler::qtHandler);

    // Configure and create Qt application object
    QCoreApplication::setOrganizationName("Fougue Ltd");
    QCoreApplication::setOrganizationDomain("www.fougue.pro");
    QCoreApplication::setApplicationName("MayoConv");
    QCoreApplication::setApplicationVersion(QString::fromUtf8(Mayo::strVersion));
    QCoreApplication app(argc, argv);
    // TODO Read settings from Mayo application
    return Mayo::runApp(&app);
}
