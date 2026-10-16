/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/math_utils.h"

#include <climits>
#include <limits>

namespace Mayo {

void TestBase::MathUtils_intRound_test()
{
    QFETCH(double, value);
    QFETCH(int, expected);

    QCOMPARE(MathUtils::intRound(value), expected);
}

void TestBase::MathUtils_intRound_test_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<int>("expected");

    QTest::newRow("zero") << 0. << 0;
    QTest::newRow("round-positive") << 2.3 << 2;
    QTest::newRow("round-negative") << -2.3 << -2;
    QTest::newRow("positive-half-away-from-zero") << 2.5 << 3;
    QTest::newRow("negative-half-away-from-zero") << -2.5 << -3;
    QTest::newRow("near-int-max") << (static_cast<double>(INT_MAX) - 0.25) << INT_MAX;
    QTest::newRow("near-int-min") << (static_cast<double>(INT_MIN) + 0.25) << INT_MIN;
    QTest::newRow("clamp-positive-overflow") << 3e9 << INT_MAX;
    QTest::newRow("clamp-negative-overflow") << -3e9 << INT_MIN;
    QTest::newRow("nan") << std::numeric_limits<double>::quiet_NaN() << 0;
    QTest::newRow("positive-infinity") << std::numeric_limits<double>::infinity() << INT_MAX;
    QTest::newRow("negative-infinity") << -std::numeric_limits<double>::infinity() << INT_MIN;
}

void TestBase::MathUtils_mappedValue_test()
{
    QFETCH(double, value);
    QFETCH(double, oldMin);
    QFETCH(double, oldMax);
    QFETCH(double, newMin);
    QFETCH(double, newMax);
    QFETCH(double, expected);

    QCOMPARE(MathUtils::mappedValue(value, oldMin, oldMax, newMin, newMax), expected);
}

void TestBase::MathUtils_mappedValue_test_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<double>("oldMin");
    QTest::addColumn<double>("oldMax");
    QTest::addColumn<double>("newMin");
    QTest::addColumn<double>("newMax");
    QTest::addColumn<double>("expected");

    QTest::newRow("map-minimum") << 0. << 0. << 10. << 0. << 100. << 0.;
    QTest::newRow("map-midpoint") << 5. << 0. << 10. << 0. << 100. << 50.;
    QTest::newRow("map-maximum") << 10. << 0. << 10. << 0. << 100. << 100.;
    QTest::newRow("map-offset-output-range") << 2.5 << 0. << 10. << -10. << 10. << -5.;
    QTest::newRow("reversed-input-range") << 2.5 << 10. << 0. << 0. << 100. << 75.;
    QTest::newRow("reversed-output-range") << 2.5 << 0. << 10. << 100. << 0. << 75.;
    QTest::newRow("extrapolate-below-input-range") << -5. << 0. << 10. << 0. << 100. << -50.;
    QTest::newRow("extrapolate-above-input-range") << 15. << 0. << 10. << 0. << 100. << 150.;
}

} // namespace Mayo
