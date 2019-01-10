/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mainwindow.h"
#include "theme.h"
#include <QtCore/QCommandLineParser>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <iostream>
#include <memory>

namespace {

class Main { Q_DECLARE_TR_FUNCTIONS(Main) };

static std::unique_ptr<Mayo::Theme> globalTheme;

struct Arguments {
    QString themeName;
    QStringList listFileToOpen;
};

static Arguments processCommandLine()
{
    Arguments args;

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

} // namespace

namespace Mayo {

// Declared in theme.h
Theme* mayoTheme() {
    return globalTheme.get();
}

} // namespace Mayo

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Fougue Ltd");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    QApplication::setApplicationVersion("0.1");

    const Arguments args = processCommandLine();

    // Create theme
    globalTheme.reset(Mayo::createTheme(args.themeName));
    if (!globalTheme) {
        const QString errorText =
                Main::tr("ERROR: Failed to load theme '%1'").arg(args.themeName);
        std::cerr << qUtf8Printable(errorText) << std::endl;
        return -1;
    }
    Mayo::mayoTheme()->setup();

    // Create MainWindow
    Mayo::MainWindow mainWindow;
    mainWindow.setWindowTitle(QApplication::applicationName());
    mainWindow.show();
    if (!args.listFileToOpen.empty()) {
        QTimer::singleShot(0, [&]{
            mainWindow.openDocumentsFromList(args.listFileToOpen);
        });
    }

    return app.exec();
}
