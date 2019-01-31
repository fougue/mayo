/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_document_item_factory.h"
#include "gpx_mesh_item.h"
#include "gpx_xde_document_item.h"

#include "mainwindow.h"
#include "theme.h"

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

    // Register Gpx factory functions
    GpxDocumentItemFactory::instance()->registerCreatorFunction(
                XdeDocumentItem::TypeName,
                &GpxDocumentItemFactory::createGpx<XdeDocumentItem, GpxXdeDocumentItem>);
    GpxDocumentItemFactory::instance()->registerCreatorFunction(
                MeshItem::TypeName,
                &GpxDocumentItemFactory::createGpx<MeshItem, GpxMeshItem>);

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
        QTimer::singleShot(0, [&]{
            mainWindow.openDocumentsFromList(args.listFileToOpen);
        });
    }

    return app->exec();
}

} // namespace Mayo

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Fougue Ltd");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    QApplication::setApplicationVersion("0.1");
    return Mayo::runApp(&app);
}
