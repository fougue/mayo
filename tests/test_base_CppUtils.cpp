/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/cpp_utils.h"

namespace Mayo {

void TestBase::CppUtils_indexInSpan_test()
{
    const std::vector<std::string> vecString = { "first", "second", "third", "fourth", "fifth" };
    const std::string& item0 = vecString.at(0);
    const std::string& item1 = vecString.at(1);
    const std::string& item2 = vecString.at(2);
    const std::string& item3 = vecString.at(3);
    const std::string& item4 = vecString.at(4);
    QCOMPARE(CppUtils::indexInSpan(vecString, item0), 0);
    QCOMPARE(CppUtils::indexInSpan(vecString, item1), 1);
    QCOMPARE(CppUtils::indexInSpan(vecString, item2), 2);
    QCOMPARE(CppUtils::indexInSpan(vecString, item3), 3);
    QCOMPARE(CppUtils::indexInSpan(vecString, item4), 4);
}

void TestBase::CppUtils_toggle_test()
{
    bool v = false;
    CppUtils::toggle(v);
    QCOMPARE(v, true);
    CppUtils::toggle(v);
    QCOMPARE(v, false);
}

} // namespace Mayo
