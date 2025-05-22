/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../app/app_module.h"
#include "../app/library_info.h"
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
#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>

#include <OpenGl_GraphicDriver.hxx>

#include <fmt/format.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>

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
    FilePath filepathUseSettings;
    FilePath filepathWriteSettings;
    FilePath filepathLog;
    std::vector<FilePath> listFilepathToExport;
    std::vector<FilePath> listFilepathToOpen;
    bool cacheUseSettings = false;
    bool includeDebugLogs = true;
    bool progressReport = true;
    bool showSystemInformation = false;
};

// Helper to filter out AppModule settings that are not useful for MayoConv application
class ExludeSettingPredicate {
public:
    ExludeSettingPredicate()
    {
        auto settings = AppModule::get()->settings();
        auto properties = AppModule::get()->properties();
        const int groupCount = settings->groupCount();
        for (int igroup = 0; igroup < groupCount; ++igroup) {
            const Settings::GroupIndex groupId{igroup};
            if (
                groupId != properties->groupId_system
                && groupId != properties->groupId_application
                && groupId != properties->groupId_graphics
               )
            {
                continue; // Skip
            }

            for (int isection = 0; isection < settings->sectionCount(groupId); ++isection) {
                const Settings::SectionIndex sectionId{groupId, isection};
                for (int isetting = 0; isetting < settings->settingCount(sectionId); ++isetting) {
                    const Settings::SettingIndex settingId{sectionId, isetting};
                    m_setExcludedProperty.insert(settings->property(settingId));
                }
            }
        }
    }

    bool fn(const Property& prop) const
    {
        return AppModule::excludeSettingPredicate(prop)
               || m_setExcludedProperty.find(&prop) != m_setExcludedProperty.cend();
    }

private:
    std::unordered_set<const Property*> m_setExcludedProperty;
};

std::ostream& operator<<(std::ostream& ostr, const QString& str)
{
    ostr << to_stdString(str);
    return ostr;
}

void showSystemInformation(std::ostream& ostr)
{
    const char indent[] = "    ";

    // Mayo version
    ostr << '\n'
         << "Mayo: v" << strVersion
         << "  commit:" << strVersionCommitId
         << "  revnum:" << versionRevisionNumber
         << "  " << QT_POINTER_SIZE * 8 << "bit"
         << '\n';

    // OS version
    ostr << '\n' << "OS: " << QSysInfo::prettyProductName()
         << " [" << QSysInfo::kernelType() << " version " << QSysInfo::kernelVersion() << "]" << '\n'
         << "Current CPU Architecture: " << QSysInfo::currentCpuArchitecture() << '\n';

    // Qt version
    ostr << '\n' << QLibraryInfo::build() << '\n';

    // OpenCascade version
    ostr << '\n' << "OpenCascade: " << OCC_VERSION_STRING_EXT << " (build)" << '\n';

    // Other registered libraries
    for (const LibraryInfo& libInfo : AppModule::get()->libraryInfoArray()) {
        ostr << '\n' << libInfo.name << ": " << libInfo.version
             << " " << libInfo.versionDetails
             << '\n';
    }

    // I/O supported formats
    {
        ostr << '\n' << "Import(read) formats:" << '\n' << indent;
        const IO::System* ioSystem = AppModule::get()->ioSystem();
        for (IO::Format format : ioSystem->readerFormats())
            ostr << IO::formatIdentifier(format) << " ";

        ostr << '\n' << "Export(write) formats:" << '\n' << indent;
        for (IO::Format format : ioSystem->writerFormats())
            ostr << IO::formatIdentifier(format) << " ";

        ostr << '\n';
    }

    ostr.flush();
}

} // namespace

// Parses command line and process Qt builtin options(basically --version and --help)
static CommandLineArguments processCommandLine()
{
    CommandLineArguments args;

    // Configure command-line parser
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                Main::tr("mayo-conv the opensource CAD converter")
    );

    const QCommandLineOption cmdShowHelp(
        QStringList{ "?", "h", "help" },
        Main::tr("Display help on commandline options")
    );
    cmdParser.addOption(cmdShowHelp);

    const QCommandLineOption cmdShowVersion(
        QStringList{ "v", "version" },
        Main::tr("Display version information")
    );
    cmdParser.addOption(cmdShowVersion);

    const QCommandLineOption cmdUseSettings(
        QStringList{ "u", "use-settings" },
        Main::tr("Use settings file(INI format) for the conversion. When this option isn't specified "
                 "then cached settings are used"),
        Main::tr("filepath")
    );
    cmdParser.addOption(cmdUseSettings);

    const QCommandLineOption cmdCacheSettings(
        QStringList{ "c", "cache-settings" },
        Main::tr("Cache settings file provided with --use-settings for further use")
    );
    cmdParser.addOption(cmdCacheSettings);

    const QCommandLineOption cmdWriteSettingsCache(
        QStringList{ "w", "write-settings-cache" },
        Main::tr("Write settings cache to an output file(INI format)"),
        Main::tr("filepath")
    );
    cmdParser.addOption(cmdWriteSettingsCache);

    const QCommandLineOption cmdFileToExport(
                QStringList{ "e", "export" },
                Main::tr("Export opened files into an output file, can be repeated for different "
                         "formats(eg. -e file.stp -e file.igs...)"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdFileToExport);

    const QCommandLineOption cmdLogFile(
                QStringList{ "log-file" },
                Main::tr("Writes log messages into output file"),
                Main::tr("filepath")
    );
    cmdParser.addOption(cmdLogFile);

    const QCommandLineOption cmdDebugLogs(
                QStringList{ "debug-logs" },
                Main::tr("Don't filter out debug log messages in release build")
    );
    cmdParser.addOption(cmdDebugLogs);

    const QCommandLineOption cmdNoProgress(
                QStringList{ "no-progress" },
                Main::tr("Disable progress reporting in console output")
    );
    cmdParser.addOption(cmdNoProgress);

    const QCommandLineOption cmdSysInfo(
                QStringList{ "system-info" },
                Main::tr("Show detailed system information and quit")
    );
    cmdParser.addOption(cmdSysInfo);

    cmdParser.addPositionalArgument(
                Main::tr("files"),
                Main::tr("Files to open(import)"),
                Main::tr("[files...]")
    );

    cmdParser.process(QCoreApplication::arguments());

    // Retrieve arguments
    if (cmdParser.isSet(cmdShowHelp))
        cmdParser.showHelp();

    if (cmdParser.isSet(cmdShowVersion))
        cmdParser.showVersion();

    if (cmdParser.isSet(cmdUseSettings))
        args.filepathUseSettings = filepathFrom(cmdParser.value(cmdUseSettings));

    if (cmdParser.isSet(cmdWriteSettingsCache))
        args.filepathWriteSettings = filepathFrom(cmdParser.value(cmdWriteSettingsCache));

    args.cacheUseSettings = cmdParser.isSet(cmdCacheSettings);

    if (cmdParser.isSet(cmdLogFile))
        args.filepathLog = filepathFrom(cmdParser.value(cmdLogFile));

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
    args.progressReport = !cmdParser.isSet(cmdNoProgress);
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

    guiApp->setAutomaticDocumentMapping(false); // GuiDocument objects aren't needed
    setFunctionCreateGraphicsDriver([]{
        return makeOccHandle<OpenGl_GraphicDriver>(GraphicsUtils::AspectDisplayConnection_create());
    });
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsShapeObjectDriver>());
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsMeshObjectDriver>());
    guiApp->addGraphicsObjectDriver(makeOccHandle<GraphicsPointCloudObjectDriver>());
}

// Initializes and runs Mayo application
static int runApp(QCoreApplication* qtApp)
{
    const CommandLineArguments args = processCommandLine();

    const ExludeSettingPredicate excludeSettingPredicate;
    auto fnExcludeSettingPredicate = [&](const Property& prop) {
        return excludeSettingPredicate.fn(prop);
    };

    // Helper function: print critical message and exit application with code failure
    auto fnCriticalExit = [](const QString& msg) {
        qCritical().noquote() << msg;
        std::exit(EXIT_FAILURE);
    };

    // Helper function: load application settings from INI file(if provided) otherwise use the
    // application regular storage(eg registry on Windows)
    auto fnLoadAppSettings = [&](Settings* appSettings) {
        if (args.filepathUseSettings.empty()) {
            appSettings->load();
        }
        else {
            const QString strFilepathSettings = filepathTo<QString>(args.filepathUseSettings);
            if (!filepathIsRegularFile(args.filepathUseSettings))
                fnCriticalExit(Main::tr("Failed to load application settings file [path=%1]").arg(strFilepathSettings));

            QSettingsStorage fileSettings(strFilepathSettings, QSettings::IniFormat);
            appSettings->loadFrom(fileSettings, fnExcludeSettingPredicate);
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
    auto app = appModule->application();
    appModule->addLibraryInfo(
        IO::AssimpLib::strName(), IO::AssimpLib::strVersion(), IO::AssimpLib::strVersionDetails()
    );
    appModule->addLibraryInfo(
        IO::GmioLib::strName(), IO::GmioLib::strVersion(), IO::GmioLib::strVersionDetails()
    );
    TextId::addTranslatorFunction(&qtAppTranslate); // Set Qt i18n backend
#ifdef MAYO_OS_WINDOWS
    initOpenCascadeEnvironment("opencascade.conf");
#endif

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

    // Application settings
    appModule->settings()->resetAll();
    fnLoadAppSettings(appModule->settings());

    // Write cached settings to ouput file if asked by user
    if (!args.filepathWriteSettings.empty()) {
        const QString strFilepathSettings = filepathTo<QString>(args.filepathWriteSettings);
        QSettingsStorage fileSettings(strFilepathSettings, QSettings::IniFormat);
        appModule->settings()->saveAs(&fileSettings, fnExcludeSettingPredicate);
        fileSettings.sync();
        if (fileSettings.get().status() != QSettings::NoError)
            fnCriticalExit(Main::tr("Error when writing to '%1'").arg(strFilepathSettings));

        qInfo().noquote() << Main::tr("Settings cache written to %1").arg(strFilepathSettings);
        return 0;
    }

    // Process CLI
     if (args.showSystemInformation) {
        showSystemInformation(std::cout);
        return EXIT_SUCCESS;
    }

    int exitCode = EXIT_SUCCESS;
    if (args.listFilepathToOpen.empty()) {
        if (!args.listFilepathToExport.empty()) {
            qCritical() << Main::tr("No input files -> nothing to export");
            exitCode = EXIT_FAILURE;
        }
    }
    else {
        QTimer::singleShot(0, qtApp, [=]{
            CliExportArgs cliArgs;
            cliArgs.progressReport = args.progressReport;
            cliArgs.filesToOpen = args.listFilepathToOpen;
            cliArgs.filesToExport = args.listFilepathToExport;
            cli_asyncExportDocuments(app, cliArgs, [=](int retcode) { qtApp->exit(retcode); });
        });
        exitCode = qtApp->exec();
    }

    if (args.cacheUseSettings) {
        if (!args.filepathUseSettings.empty()) {
            appModule->settings()->save();
            qInfo().noquote() << Main::tr("Settings '%1' cached").arg(filepathTo<QString>(args.filepathUseSettings));
        }
        else {
            qWarning().noquote() << Main::tr("No supplied settings to cache");
        }
    }

    return exitCode;
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
    return Mayo::runApp(&app);
}
