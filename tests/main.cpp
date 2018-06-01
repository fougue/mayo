#include "test.h"

#include <QtCore/QCoreApplication>

int main(int argc, char** argv)
{
    //QCoreApplication app(argv, argv);
    Mayo::Test test;
    return QTest::qExec(&test, argc, argv);
}
