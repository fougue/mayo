/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application.h"
#include "../base/document_tree_node_properties_provider.h"
#include "../base/io_system.h"
#include "../base/settings.h"
#include "../io_gmio/io_gmio.h"
#include "../io_occ/io_occ.h"
#include "../gui/gui_application.h"
#include "../graphics/graphics_object_driver.h"
#include "app_module.h"
#include "document_tree_node_properties_providers.h"
#include "mainwindow.h"
#include "theme.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_xde.h"
#include "widget_model_tree_builder_mesh.h"

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <memory>

namespace Mayo {

class Main { Q_DECLARE_TR_FUNCTIONS(Mayo::Main) };

struct CommandLineArguments {
    QString themeName;
    std::vector<FilePath> listFilepathToOpen;
    FilePath filepathSettings;
};

static CommandLineArguments processCommandLine()
{
    CommandLineArguments args;

    // Configure command-line parser
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                Main::tr("Mayo, an open-source 3D viewer based on Qt5/OpenCascade"));
    cmdParser.addHelpOption();

    const QCommandLineOption cmdOptionTheme(
                "theme",
                Main::tr("Theme for the UI(classic|dark)"),
                Main::tr("name"));
    cmdParser.addOption(cmdOptionTheme);

    cmdParser.addPositionalArgument(
                Main::tr("files"),
                Main::tr("Files to open at startup, optionally"),
                Main::tr("[files...]"));

    const QCommandLineOption cmdFileSettings(
                "settings",
                Main::tr("Settings file(INI format) to load at startup"),
                Main::tr("filepath"));
    cmdParser.addOption(cmdFileSettings);

    cmdParser.process(QCoreApplication::arguments());

    // Retrieve arguments
    args.themeName = "dark";
    if (cmdParser.isSet(cmdOptionTheme))
        args.themeName = cmdParser.value(cmdOptionTheme);

    if (cmdParser.isSet(cmdFileSettings))
        args.filepathSettings = filepathFrom(cmdParser.value(cmdFileSettings));

    for (const QString& posArg : cmdParser.positionalArguments())
        args.listFilepathToOpen.push_back(filepathFrom(posArg));

    return args;
}

static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    const QByteArray localMsg = msg.toLocal8Bit();
//    const char* file = context.file ? context.file : "";
//    const char* function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        std::cout << "DEBUG: " << localMsg.constData() << std::endl;
        break;
    case QtInfoMsg:
        std::cout << "INFO: " << localMsg.constData() << std::endl;
        break;
    case QtWarningMsg:
        std::cerr << "WARNING: " << localMsg.constData() << std::endl;
        break;
    case QtCriticalMsg:
        std::cerr << "CRITICAL: " << localMsg.constData() << std::endl;
        break;
    case QtFatalMsg:
        std::cerr << "FATAL: " << localMsg.constData() << std::endl;
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
    // Register Graphics/TreeNode mapping drivers
    guiApp->graphicsTreeNodeMappingDriverTable()->addDriver(
                std::make_unique<GraphicsShapeTreeNodeMappingDriver>());

    // Register Graphics entity drivers
    guiApp->graphicsObjectDriverTable()->addDriver(std::make_unique<GraphicsShapeObjectDriver>());
    guiApp->graphicsObjectDriverTable()->addDriver(std::make_unique<GraphicsMeshObjectDriver>());
}

// Initializes and runs Mayo application
static int runApp(QApplication* qtApp)
{
    const CommandLineArguments args = processCommandLine();

    initBase(qtApp);
    auto app = Application::instance().get();
    auto guiApp = new GuiApplication(app);
    initGui(guiApp);

    // Register AppModule
    auto appModule = new AppModule(app);
    QObject::connect(
                guiApp, &GuiApplication::guiDocumentErased,
                appModule, &AppModule::recordRecentFileThumbnail);

    // Register WidgetModelTreeBuilter prototypes
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Mesh>());
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Xde>());

    // Create theme
    globalTheme.reset(createTheme(args.themeName));
    if (!globalTheme) {
        qCritical().noquote() << Main::tr("Failed to load theme '%1'").arg(args.themeName);
        return -1;
    }
    mayoTheme()->setup();

    // Create MainWindow
    app->settings()->loadProperty(app->settings()->findProperty(&appModule->recentFiles));
    MainWindow mainWindow(guiApp);
    mainWindow.setWindowTitle(QApplication::applicationName());
    mainWindow.show();
    if (!args.listFilepathToOpen.empty()) {
        QTimer::singleShot(0, [&]{ mainWindow.openDocumentsFromList(args.listFilepathToOpen); });
    }

    app->settings()->resetAll();
    if (args.filepathSettings.empty()) {
        app->settings()->load();
    }
    else {
        const QString strFilepathSettings = filepathTo<QString>(args.filepathSettings);
        if (!std::filesystem::is_regular_file(args.filepathSettings)) {
            qCritical().noquote() << Main::tr("Failed to load settings file '%1'").arg(strFilepathSettings);
            return -1;
        }

        QSettings fileSettings(strFilepathSettings, QSettings::IniFormat);
        app->settings()->loadFrom(fileSettings, &AppModule::excludeSettingPredicate);
    }

    const int code = qtApp->exec();
    appModule->recordRecentFileThumbnails(guiApp);
    app->settings()->save();
    return code;
}

} // namespace Mayo

int main(int argc, char* argv[])
{
    qInstallMessageHandler(&Mayo::qtMessageHandler);
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Fougue Ltd");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    return Mayo::runApp(&app);
}
