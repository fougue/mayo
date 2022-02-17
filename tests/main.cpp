/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_base.h"
#include "test_app.h"

#include <QtGui/QGuiApplication>

#include <memory>
#include <vector>

int main(int argc, char** argv)
{
    // Required by TestApp
    QGuiApplication guiApp(argc, argv);

    int retcode = 0;
    std::vector<std::unique_ptr<QObject>> vecTest;
    vecTest.emplace_back(new Mayo::TestBase);
    vecTest.emplace_back(new Mayo::TestApp);
    for (const std::unique_ptr<QObject>& test : vecTest)
        retcode += QTest::qExec(test.get(), argc, argv);

    return retcode;
}
