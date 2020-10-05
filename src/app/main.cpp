/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application.h"
#include "../base/document_tree_node_properties_provider.h"
#include "../base/io.h"
#include "../base/io_occ.h"
#include "../base/settings.h"
#include "../gui/gui_application.h"
#include "../graphics/graphics_entity_driver.h"
#include "../graphics/graphics_entity_driver_table.h"
#include "app_module.h"
#include "document_tree_node_properties_providers.h"
#include "mainwindow.h"
#include "theme.h"
#include "version.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_xde.h"
#include "widget_model_tree_builder_mesh.h"

#include <QtCore/QtDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <iostream>
#include <memory>

namespace Mayo {

class Main { Q_DECLARE_TR_FUNCTIONS(Main) };

struct CommandLineArguments {
    QString themeName;
    QStringList listFileToOpen;
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

    cmdParser.process(QApplication::arguments());

    // Retrieve arguments
    args.themeName = "dark";
    if (cmdParser.isSet(cmdOptionTheme))
        args.themeName = cmdParser.value(cmdOptionTheme);

    args.listFileToOpen = cmdParser.positionalArguments();

    return args;
}

static std::unique_ptr<Theme> globalTheme;

// Declared in theme.h
Theme* mayoTheme()
{
    return globalTheme.get();
}

static int runApp(QApplication* app)
{
    const CommandLineArguments args = processCommandLine();
    Application::setOpenCascadeEnvironment("opencascade.conf");

    // Register IO OpenCascade objects
    auto ioSystem = Application::instance()->ioSystem();
    ioSystem->addFactoryReader(std::make_unique<IO::OccFactoryReader>());
    ioSystem->addFactoryWriter(std::make_unique<IO::OccFactoryWriter>());
    IO::addPredefinedFormatProbes(ioSystem);

    // Register Graphics entity drivers
    GraphicsEntityDriverTable::instance()->addDriver(std::make_unique<GraphicsMeshEntityDriver>());
    GraphicsEntityDriverTable::instance()->addDriver(std::make_unique<GraphicsShapeEntityDriver>());

    // Register Graphics/TreeNode mapping drivers
    GuiApplication::instance()->addGraphicsTreeNodeMappingDriver(
                std::make_unique<GraphicsShapeTreeNodeMappingDriver>());

    // Register AppModule
    auto appModule = new AppModule(Application::instance().get());

    // Register document tree node providers
    DocumentTreeNodePropertiesProviderTable::instance()->addProvider(
                std::make_unique<XCaf_DocumentTreeNodePropertiesProvider>());
    DocumentTreeNodePropertiesProviderTable::instance()->addProvider(
                std::make_unique<Mesh_DocumentTreeNodePropertiesProvider>());

    // Register WidgetModelTreeBuilter prototypes
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Mesh>());
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Xde>());

    // Create theme
    globalTheme.reset(createTheme(args.themeName));
    if (!globalTheme) {
        const QString errorText = Main::tr("ERROR: Failed to load theme '%1'").arg(args.themeName);
        std::cerr << qUtf8Printable(errorText) << std::endl;
        return -1;
    }
    mayoTheme()->setup();

    // Load translation files before UI creation
    {
        auto settings = Application::instance()->settings();
        settings->loadProperty(settings->findProperty(&appModule->language));
        const QString qmFilePath = AppModule::qmFilePath(appModule->language.name());
        auto translator = new QTranslator(app);
        if (translator->load(qmFilePath))
            app->installTranslator(translator);
        else
            std::cerr << qUtf8Printable(Main::tr("Failed to load translation for '%1'").arg(qmFilePath)) << std::endl;
    }

    // Create MainWindow
    MainWindow mainWindow;
    mainWindow.setWindowTitle(QApplication::applicationName());
    mainWindow.show();
    if (!args.listFileToOpen.empty()) {
        QTimer::singleShot(0, [&]{ mainWindow.openDocumentsFromList(args.listFileToOpen); });
    }

    Application::instance()->settings()->resetAll();
    Application::instance()->settings()->load();
    const int code = app->exec();
    Application::instance()->settings()->save();
    return code;
}

} // namespace Mayo

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Fougue Ltd");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    return Mayo::runApp(&app);
}
