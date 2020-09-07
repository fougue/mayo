/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application.h"
#include "../base/document_tree_node_properties_provider.h"
#include "../base/settings.h"
#include "../base/settings_keys.h"
#include "../base/unit_system.h"
#include "../gui/gui_application.h"
#include "../graphics/graphics_entity_driver.h"
#include "../graphics/graphics_entity_driver_table.h"
#include "document_tree_node_properties_providers.h"
#include "mainwindow.h"
#include "theme.h"
#include "version.h"
#include "widget_model_tree.h"
#include "widget_model_tree_builder_xde.h"
#include "widget_model_tree_builder_mesh.h"

#include <QtCore/QCommandLineParser>
#include <QtCore/QTimer>
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

    // Register Graphics entity drivers
    GraphicsEntityDriverTable::instance()->addDriver(std::make_unique<GraphicsMeshEntityDriver>());
    GraphicsEntityDriverTable::instance()->addDriver(std::make_unique<GraphicsShapeEntityDriver>());

    // Register Graphics/TreeNode mapping drivers
    GuiApplication::instance()->addGraphicsTreeNodeMappingDriver(
                std::make_unique<GraphicsShapeTreeNodeMappingDriver>());

    // Register document tree node providers
    DocumentTreeNodePropertiesProviderTable::instance()->addProvider(
                std::make_unique<XCaf_DocumentTreeNodePropertiesProvider>());
    DocumentTreeNodePropertiesProviderTable::instance()->addProvider(
                std::make_unique<Mesh_DocumentTreeNodePropertiesProvider>());

    // Register WidgetModelTreeBuilter prototypes
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Mesh>());
    WidgetModelTree::addPrototypeBuilder(std::make_unique<WidgetModelTreeBuilder_Xde>());

    // Default values
    auto settings = Application::instance()->settings();
    settings->setDefaultValue(Keys::App_RecentFiles, QStringList());
    settings->setDefaultValue(Keys::App_MainWindowLastOpenDir, QString());
    settings->setDefaultValue(Keys::App_MainWindowLastSelectedFilter, QString());
    settings->setDefaultValue(Keys::App_MainWindowLinkWithDocumentSelector, false);
    settings->setDefaultValue(Keys::Base_StlIoLibrary, static_cast<int>(IO::StlIoLibrary::OpenCascade));
    settings->setDefaultValue(Keys::Base_UnitSystemSchema, UnitSystem::SI);
    settings->setDefaultValue(Keys::Base_UnitSystemDecimals, 2);
    settings->setDefaultValue(Keys::Gpx_MeshDefaultColor, QColor(255, 228, 196));
    settings->setDefaultValue(Keys::Gpx_MeshDefaultMaterial, Graphic3d_NOM_PLASTIC);
    settings->setDefaultValue(Keys::Gpx_MeshDefaultShowEdges, false);
    settings->setDefaultValue(Keys::Gpx_MeshDefaultShowNodes, false);
    settings->setDefaultValue(Keys::Gui_ClipPlaneCappingHatch, Aspect_HS_SOLID);
    settings->setDefaultValue(Keys::Gui_ClipPlaneCappingOn, true);
    settings->setDefaultValue(Keys::Gui_DefaultShowOriginTrihedron, true);

    {
        auto fnUpdateDefaults = [=]{
            GraphicsMeshEntityDriver::DefaultValues defaults;
            defaults.showEdges = settings->valueAs<bool>(Keys::Gpx_MeshDefaultShowEdges);
            defaults.showNodes = settings->valueAs<bool>(Keys::Gpx_MeshDefaultShowNodes);
            defaults.color = settings->valueAs<QColor>(Keys::Gpx_MeshDefaultColor);
            defaults.material = settings->valueAsEnum<Graphic3d_NameOfMaterial>(Keys::Gpx_MeshDefaultMaterial);
            GraphicsMeshEntityDriver::setDefaultValues(defaults);
        };
        fnUpdateDefaults();
        auto settings = Application::instance()->settings();
        QObject::connect(settings, &Settings::valueChanged, [=](const QString& key) {
            if (key == Keys::Gpx_MeshDefaultShowEdges
                    || key == Keys::Gpx_MeshDefaultShowNodes
                    || key == Keys::Gpx_MeshDefaultColor
                    || key == Keys::Gpx_MeshDefaultMaterial)
            {
                fnUpdateDefaults();
            }
        });
    }

    // Create theme
    globalTheme.reset(createTheme(args.themeName));
    if (!globalTheme) {
        const QString errorText =
                Main::tr("ERROR: Failed to load theme '%1'").arg(args.themeName);
        std::cerr << qUtf8Printable(errorText) << std::endl;
        return -1;
    }
    mayoTheme()->setup();

    // Create MainWindow
    MainWindow mainWindow;
    mainWindow.setWindowTitle(QApplication::applicationName());
    mainWindow.show();
    if (!args.listFileToOpen.empty()) {
        QTimer::singleShot(0, [&]{ mainWindow.openDocumentsFromList(args.listFileToOpen); });
    }

    return app->exec();
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
