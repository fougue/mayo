/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_base.h"
#include "test_measure.h"
#include "test_app.h"

#include <cstring>
#include <memory>
#include <vector>

namespace Mayo {

int runTests(int argc, char* argv[])
{
    QStringList args;
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--runtests") != 0)
            args.push_back(QString::fromUtf8(argv[i]));
    }

    int retcode = 0;
    std::vector<std::unique_ptr<QObject>> vecTest;
    vecTest.emplace_back(new Mayo::TestBase);
    vecTest.emplace_back(new Mayo::TestMeasure);
    vecTest.emplace_back(new Mayo::TestApp);
    for (const std::unique_ptr<QObject>& test : vecTest)
        retcode += QTest::qExec(test.get(), args);

    return retcode;
}

} // namespace Mayo
