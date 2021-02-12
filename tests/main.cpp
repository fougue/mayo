/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test.h"

#include <memory>
#include <vector>

int main(int argc, char** argv)
{
    int retcode = 0;
    std::vector<std::unique_ptr<QObject>> vecTest;
    vecTest.emplace_back(new Mayo::Test);
    for (const std::unique_ptr<QObject>& test : vecTest)
        retcode += QTest::qExec(test.get(), argc, argv);

    return retcode;
}
