#include "mainwindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setOrganizationName("Fougue");
    QApplication::setOrganizationDomain("www.fougue.pro");
    QApplication::setApplicationName("Mayo");
    QApplication::setApplicationVersion("0.1");

    Mayo::MainWindow w;
    w.show();

    return a.exec();
}
