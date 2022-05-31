/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_base.h"
#include "test_app.h"

#include <memory>
#include <vector>

namespace Mayo {

int runTests(const QStringList& args)
{
    int retcode = 0;
    std::vector<std::unique_ptr<QObject>> vecTest;
    vecTest.emplace_back(new Mayo::TestBase);
    vecTest.emplace_back(new Mayo::TestApp);
    for (const std::unique_ptr<QObject>& test : vecTest)
        retcode += QTest::qExec(test.get(), args);

    return retcode;
}

} // namespace Mayo
