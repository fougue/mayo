#include "gui_application.h"
#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Fougue");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    QApplication::setApplicationVersion("0.1");

    Mayo::GuiApplication guiApp;
    Mayo::MainWindow w(&guiApp);
    w.show();

    return app.exec();
}
