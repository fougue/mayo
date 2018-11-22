/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test.h"

#include <QtCore/QCoreApplication>

int main(int argc, char** argv)
{
    //QCoreApplication app(argv, argv);
    Mayo::Test test;
    return QTest::qExec(&test, argc, argv);
}
