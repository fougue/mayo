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
#include "../io_gmio/io_gmio.h"
#include "../io_occ/io_occ.h"
#include "../graphics/graphics_object_driver.h"
#include "../gui/gui_application.h"
#include "app_module.h"
#include "console.h"
#include "document_tree_node_properties_providers.h"
#include "mainwindow.h"
#include "theme.h"
#include "version.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_mesh.h"
#include "widget_model_tree_builder_xde.h"

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>

#include <Message.hxx>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <memory>
#include <unordered_map>

#ifdef Q_OS_WIN
#  include <windows.h> // For AttachConsole(), etc.
#endif

namespace Mayo {

class Main { Q_DECLARE_TR_FUNCTIONS(Mayo::Main) };

struct CommandLineArguments {
    QString themeName;
    FilePath filepathSettings;
    std::vector<FilePath> listFilepathToExport;
    std::vector<FilePath> listFilepathToOpen;
    bool cliProgressReport = true;
};

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

    if (cmdParser.isSet(cmdFileToExport)) {
        for (const QString& strFilepath : cmdParser.values(cmdFileToExport))
            args.listFilepathToExport.push_back(filepathFrom(strFilepath));
    }

    for (const QString& posArg : cmdParser.positionalArguments())
        args.listFilepathToOpen.push_back(filepathFrom(posArg));

    args.cliProgressReport = !cmdParser.isSet(cmdCliNoProgress);

    return args;
}

static void qtMessageHandler(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg)
{
    const std::string localMsg = consoleToPrintable(msg);
//    const char* file = context.file ? context.file : "";
//    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
#ifndef NDEBUG
        std::cout << "DEBUG: " << localMsg << std::endl;
#endif
        break;
    case QtInfoMsg:
        std::cout << "INFO: " << localMsg << std::endl;
        break;
    case QtWarningMsg:
        std::cerr << "WARNING: " << localMsg << std::endl;
        break;
    case QtCriticalMsg:
        std::cerr << "CRITICAL: " << localMsg << std::endl;
        break;
    case QtFatalMsg:
        std::cerr << "FATAL: " << localMsg << std::endl;
        break;
    }
}

static std::unique_ptr<Theme> globalTheme;

// Declared in theme.h
Theme* mayoTheme()
{
    return globalTheme.get();
}

// Initializes "Base" objects
static void initBase(QCoreApplication* qtApp)
{
    Application::setOpenCascadeEnvironment("opencascade.conf");
    auto app = Application::instance();

    // Load translation files
    {
        const QString qmFilePath = AppModule::qmFilePath(AppModule::languageCode(app));
        auto translator = new QTranslator(app.get());
        if (translator->load(qmFilePath))
            qtApp->installTranslator(translator);
        else
            qWarning() << Main::tr("Failed to load translation for '%1'").arg(qmFilePath);
    }

    // Register I/O objects
    app->ioSystem()->addFactoryReader(std::make_unique<IO::OccFactoryReader>());
    app->ioSystem()->addFactoryWriter(std::make_unique<IO::OccFactoryWriter>());
    app->ioSystem()->addFactoryWriter(IO::GmioFactoryWriter::create());
    IO::addPredefinedFormatProbes(app->ioSystem());

    // Register providers to query document tree node properties
    app->documentTreeNodePropertiesProviderTable()->addProvider(
                std::make_unique<XCaf_DocumentTreeNodePropertiesProvider>());
    app->documentTreeNodePropertiesProviderTable()->addProvider(
                std::make_unique<Mesh_DocumentTreeNodePropertiesProvider>());
}

// Initializes "GUI" objects
static void initGui(GuiApplication* guiApp)
{
    if (!guiApp)
        return;

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
    struct ErrorMessageCollect : public Messenger {
        QString message;
        void emitMessage(MessageType msgType, const QString& text) override {
            if (msgType == MessageType::Error)
                message += text + " ";
        }
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
            const std::string strMessage = consoleToPrintable(taskMgr->title(taskId).replace('\n', ' '));
            int lineWidth = strMessage.size();
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
            qInfo() << taskMgr->title(taskId);
    });
    QObject::connect(taskMgr, &TaskManager::ended, app, [=](TaskId taskId) {
        if (args.cliProgressReport) {
            fnPrintProgress();
        }
        else {
            if (helper->mapTaskStatus.at(taskId)->success)
                qInfo() << taskMgr->title(taskId);
            else
                qCritical() << taskMgr->title(taskId);
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
                .withEntityPostProcessInfoProgress(20, Main::tr("Mesh BRep shapes"))
                .withMessenger(&errorCollect)
                .withTaskProgress(progress)
                .execute();
            taskMgr->setTitle(progress->taskId(), okImport ? Main::tr("Imported") : errorCollect.message);
            helper->mapTaskStatus.at(progress->taskId())->success = okImport;
            helper->mapTaskStatus.at(progress->taskId())->finished = true;
    });
    helper->mapTaskStatus.insert({ importTaskId, std::make_unique<TaskStatus>() });
    taskMgr->setTitle(importTaskId, Main::tr("Importing..."));
    taskMgr->exec(importTaskId, TaskAutoDestroy::Off);
    if (!okImport)
        return fnExit(EXIT_FAILURE); // Error

    // Run export operations(asynchronous)
    for (const FilePath& filepath : args.listFilepathToExport) {
        const QString strFilename = filepathTo<QString>(filepath.filename());
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
                const QString msg = okExport ? Main::tr("Exported %1").arg(strFilename) : errorCollect.message;
                taskMgr->setTitle(progress->taskId(), msg);
                helper->mapTaskStatus.at(progress->taskId())->success = okExport;
                helper->mapTaskStatus.at(progress->taskId())->finished = true;
                --(helper->exportTaskCount);
        });
        helper->mapTaskStatus.insert({ taskId, std::make_unique<TaskStatus>() });
        taskMgr->setTitle(taskId, Main::tr("Exporting %1...").arg(strFilename));
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
                fnCriticalExit(Main::tr("Failed to load settings file '%1'").arg(strFilepathSettings));

            QSettings fileSettings(strFilepathSettings, QSettings::IniFormat);
            appSettings->loadFrom(fileSettings, &AppModule::excludeSettingPredicate);
        }
    };

    // Initialize Base application
    initBase(qtApp);
    auto app = Application::instance().get();

    // Register AppModule
    auto appModule = new AppModule(app);
    app->settings()->setPropertyValueConversion(*appModule);

    // Process CLI
    if (!args.listFilepathToExport.empty()) {
        if (args.listFilepathToOpen.empty())
            fnCriticalExit(Main::tr("No input files -> nothing to export"));

        app->settings()->resetAll();
        fnLoadAppSettings(app->settings());
        QTimer::singleShot(0, qtApp, [=]{
            cli_asyncExportDocuments(app, args, [=](int retcode) { qtApp->exit(retcode); });
        });
        return qtApp->exec();
    }

    // Initialize Gui application
    auto guiApp = new GuiApplication(app);
    initGui(guiApp);
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

    // Create MainWindow
    app->settings()->loadProperty(app->settings()->findProperty(&appModule->recentFiles));
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
    qInstallMessageHandler(&Mayo::qtMessageHandler);
    qAddPostRoutine(&Mayo::onQtAppExit);

    auto fnArgEqual = [](const char* arg, const char* option) { return std::strcmp(arg, option) == 0; };
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (fnArgEqual(arg, "-e") || fnArgEqual(arg, "--export")
                || fnArgEqual(arg, "-h") || fnArgEqual(arg, "--help")
                || fnArgEqual(arg, "-v") || fnArgEqual(arg, "--version"))
        {
            Mayo::isAppCliMode = true;
            break;
        }
    }

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
