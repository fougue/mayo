/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/quantity.h"
#include "../src/base/unit.h"
#include "../src/base/unit_system.h"

#include <cmath>
#include <cstring>

Q_DECLARE_METATYPE(Mayo::UnitSystem::TranslateResult)

namespace Mayo {

// For the sake of QCOMPARE()
bool operator==(const UnitSystem::TranslateResult& lhs, const UnitSystem::TranslateResult& rhs)
{
    return std::abs(lhs.value - rhs.value) < 1e-6
           && std::strcmp(lhs.strUnit, rhs.strUnit) == 0
           && std::abs(lhs.factor - rhs.factor) < 1e-6
        ;
}

void TestBase::UnitSystem_test()
{
    QFETCH(UnitSystem::TranslateResult, trResultActual);
    QFETCH(UnitSystem::TranslateResult, trResultExpected);
    QCOMPARE(trResultActual, trResultExpected);
}

void TestBase::UnitSystem_test_data()
{
    QTest::addColumn<UnitSystem::TranslateResult>("trResultActual");
    QTest::addColumn<UnitSystem::TranslateResult>("trResultExpected");

    const UnitSystem::Schema schemaSI = UnitSystem::SI;
    QTest::newRow("80mm")
        << UnitSystem::translate(schemaSI, 80 * Quantity_Millimeter)
        << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8cm")
        << UnitSystem::translate(schemaSI, 8 * Quantity_Centimeter)
        << UnitSystem::TranslateResult{ 80., "mm", 1. };
    QTest::newRow("8m")
        << UnitSystem::translate(schemaSI, 8 * Quantity_Meter)
        << UnitSystem::TranslateResult{ 8000., "mm", 1. };
    QTest::newRow("50mm²")
        << UnitSystem::translate(schemaSI, 0.5 * Quantity_SquareCentimeter)
        << UnitSystem::TranslateResult{ 50., "mm²", 1. };
    QTest::newRow("50kg/m³")
        << UnitSystem::translate(schemaSI, 25 * Quantity_KilogramPerCubicMeter)
        << UnitSystem::TranslateResult{ 25., "kg/m³", 1. };
    QTest::newRow("40kg/m³")
        << UnitSystem::translate(schemaSI, 0.04 * Quantity_GramPerCubicCentimeter)
        << UnitSystem::TranslateResult{ 40., "kg/m³", 1. };

    constexpr double radDeg = Quantity_Degree.value();
    QTest::newRow("degrees(PIrad)")
        << UnitSystem::degrees(3.14159265358979323846 * Quantity_Radian)
        << UnitSystem::TranslateResult{ 180., "°", radDeg };

    QTest::newRow("time(1s)")
        << UnitSystem::milliseconds(1 * Quantity_Second)
        << UnitSystem::TranslateResult{ 1000., "ms", Quantity_Millisecond.value() };
    QTest::newRow("time(5s)")
        << UnitSystem::milliseconds(5 * Quantity_Second)
        << UnitSystem::TranslateResult{ 5000., "ms", Quantity_Millisecond.value() };
    QTest::newRow("time(2min)")
        << UnitSystem::milliseconds(2 * Quantity_Minute)
        << UnitSystem::TranslateResult{ 2 * 60 * 1000., "ms", Quantity_Millisecond.value() };
}

} // namespace Mayo
