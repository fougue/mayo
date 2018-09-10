/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mainwindow.h"
#include "theme.h"
#include <QtCore/QCommandLineParser>
#include <QtWidgets/QApplication>
#include <iostream>
#include <memory>

namespace {

class Main { Q_DECLARE_TR_FUNCTIONS(Main) };

static std::unique_ptr<Mayo::Theme> globalTheme;

bool processCommandLine()
{
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                Main::tr("Mayo, an Qt5/OpenCascade 3D viewer"));
    cmdParser.addHelpOption();

    const QCommandLineOption cmdOptionTheme(
                "theme",
                Main::tr("Theme for the UI(classic|dark)"),
                Main::tr("name"));
    cmdParser.addOption(cmdOptionTheme);

    cmdParser.process(QApplication::arguments());

    QString themeName("dark");
    if (cmdParser.isSet(cmdOptionTheme))
        themeName = cmdParser.value(cmdOptionTheme);
    globalTheme.reset(Mayo::createTheme(themeName));
    if (!globalTheme) {
        std::cerr << qUtf8Printable(Main::tr("Unknown theme '%1'").arg(themeName))
                  << std::endl;
        return false;
    }

    return true;
}

} // namespace

namespace Mayo {

Theme* mayoTheme() {
    return globalTheme.get();
}

} // namespace Mayo

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Fougue");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    QApplication::setApplicationVersion("0.1");

    if (!processCommandLine())
        return -1;

    Mayo::mayoTheme()->setup();
    Mayo::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
