/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/geom_utils.h"
#include "../src/base/occ_handle.h"

#include <GeomAdaptor_Curve.hxx>
#include <Geom_Circle.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>

#include <cmath>

namespace Mayo {

namespace {

GeomAdaptor_Curve unitCircleCurve()
{
    auto circle = makeOccHandle<Geom_Circle>(gp_Ax2{}, 1.);
    return GeomAdaptor_Curve(circle, 0., 2. * std::acos(-1.));
}

} // namespace

void TestBase::GeomUtils_normalizedU_test()
{
    QFETCH(double, u);
    QFETCH(double, expected);

    auto circle = makeOccHandle<Geom_Circle>(gp_Ax2{}, 1.);
    const GeomAdaptor_Curve curve(circle, 2., 6.);
    QCOMPARE(GeomUtils::normalizedU(curve, u), expected);
}

void TestBase::GeomUtils_normalizedU_test_data()
{
    QTest::addColumn<double>("u");
    QTest::addColumn<double>("expected");

    QTest::newRow("first-parameter") << 2. << 0.;
    QTest::newRow("midpoint") << 4. << 0.5;
    QTest::newRow("last-parameter") << 6. << 1.;
    QTest::newRow("below-range") << 1. << -0.25;
    QTest::newRow("above-range") << 7. << 1.25;
}

void TestBase::GeomUtils_d0_test()
{
    QFETCH(double, u);
    QFETCH(double, x);
    QFETCH(double, y);

    const GeomAdaptor_Curve curve = unitCircleCurve();
    const gp_Pnt expected{x, y, 0.};
    QVERIFY(GeomUtils::equal(GeomUtils::d0(curve, u), expected, 1e-12));
}

void TestBase::GeomUtils_d0_test_data()
{
    QTest::addColumn<double>("u");
    QTest::addColumn<double>("x");
    QTest::addColumn<double>("y");

    QTest::newRow("start") << 0. << 1. << 0.;
    QTest::newRow("quarter-turn") << (std::acos(-1.) / 2.) << 0. << 1.;
}

void TestBase::GeomUtils_d1_test()
{
    QFETCH(double, u);
    QFETCH(double, x);
    QFETCH(double, y);

    const GeomAdaptor_Curve curve = unitCircleCurve();
    const gp_Vec expected{x, y, 0.};
    QVERIFY(GeomUtils::equal(GeomUtils::d1(curve, u), expected, 1e-12, 1e-12));
}

void TestBase::GeomUtils_d1_test_data()
{
    QTest::addColumn<double>("u");
    QTest::addColumn<double>("x");
    QTest::addColumn<double>("y");

    QTest::newRow("start-tangent") << 0. << 0. << 1.;
    QTest::newRow("quarter-turn-tangent") << (std::acos(-1.) / 2.) << -1. << 0.;
}

void TestBase::GeomUtils_d0d1_test()
{
    QFETCH(double, u);
    QFETCH(double, pointX);
    QFETCH(double, pointY);
    QFETCH(double, tangentX);
    QFETCH(double, tangentY);

    const GeomAdaptor_Curve curve = unitCircleCurve();
    const auto [point, tangent] = GeomUtils::d0d1(curve, u);
    QVERIFY(GeomUtils::equal(point, gp_Pnt{pointX, pointY, 0.}, 1e-12));
    QVERIFY(GeomUtils::equal(tangent, gp_Vec{tangentX, tangentY, 0.}, 1e-12, 1e-12));
}

void TestBase::GeomUtils_d0d1_test_data()
{
    QTest::addColumn<double>("u");
    QTest::addColumn<double>("pointX");
    QTest::addColumn<double>("pointY");
    QTest::addColumn<double>("tangentX");
    QTest::addColumn<double>("tangentY");

    QTest::newRow("start") << 0. << 1. << 0. << 0. << 1.;
    QTest::newRow("quarter-turn") << (std::acos(-1.) / 2.) << 0. << 1. << -1. << 0.;
}

void TestBase::GeomUtils_makeTranslation_test()
{
    {
        const gp_Vec translation{3., -2., 5.};
        const gp_Pnt transformed = gp::Origin().Transformed(GeomUtils::makeTranslation(translation));
        QVERIFY(transformed.Distance(gp_Pnt{3., -2., 5.}) < 1e-12);
    }

    {
        const gp_Pnt source{1., 2., 3.};
        const gp_Pnt target{-4., 5., 7.};
        const gp_Trsf trsf = GeomUtils::makeTranslation(source, target);
        QVERIFY(source.Transformed(trsf).Distance(target) < 1e-12);
        const gp_Pnt translatedOther = gp_Pnt{2., 3., 4.}.Transformed(trsf);
        QVERIFY(translatedOther.Distance(gp_Pnt{-3., 6., 8.}) < 1e-12);
    }
}

void TestBase::GeomUtils_makeRotation_test()
{
    const gp_Ax1 axis{gp::Origin(), gp::DZ()};
    const gp_Trsf trsf = GeomUtils::makeRotation(axis, std::acos(-1.) / 2.);
    const gp_Pnt rotated = gp_Pnt{1., 0., 0.}.Transformed(trsf);
    QVERIFY(rotated.Distance(gp_Pnt{0., 1., 0.}) < 1e-12);
}

void TestBase::GeomUtils_makeMirror_test()
{
    const gp_Trsf trsf = GeomUtils::makeMirror(gp_Ax2{});
    const gp_Pnt mirroredOffPlane = gp_Pnt{1., 2., 3.}.Transformed(trsf);
    const gp_Pnt mirroredOnPlane = gp_Pnt{1., 2., 0.}.Transformed(trsf);
    QVERIFY(mirroredOffPlane.Distance(gp_Pnt{1., 2., -3.}) < 1e-12);
    QVERIFY(mirroredOnPlane.Distance(gp_Pnt{1., 2., 0.}) < 1e-12);
}

void TestBase::GeomUtils_equalPoint_test()
{
    const gp_Pnt point{1., 2., 3.};
    QVERIFY(GeomUtils::equal(point, point));
    QVERIFY(GeomUtils::equal(point, gp_Pnt{1.0001, 2., 3.}, 0.001));
    QVERIFY(!GeomUtils::equal(point, gp_Pnt{1.0001, 2., 3.}, 0.00001));
}

void TestBase::GeomUtils_equalVector_test()
{
    const gp_Vec vector{1., 0., 0.};
    QVERIFY(GeomUtils::equal(vector, vector));

    // The linear tolerance controls magnitude differences
    QVERIFY(GeomUtils::equal(vector, gp_Vec{1.0001, 0., 0.}, 0.001, 1e-8));
    QVERIFY(!GeomUtils::equal(vector, gp_Vec{1.0001, 0., 0.}, 0.00001, 1e-8));

    // The angular tolerance controls direction differences
    const double angle = 0.001;
    const gp_Vec rotated{std::cos(angle), std::sin(angle), 0.};
    QVERIFY(GeomUtils::equal(vector, rotated, 1e-12, 0.002));
    QVERIFY(!GeomUtils::equal(vector, rotated, 1e-12, 0.0001));
}

void TestBase::GeomUtils_isNull_test()
{
    QFETCH(double, x);
    QFETCH(double, sqrLinearTol);
    QFETCH(bool, expected);

    QVERIFY(GeomUtils::isNull(gp_Vec{x, 0., 0.}, sqrLinearTol) == expected);
}

void TestBase::GeomUtils_isNull_test_data()
{
    QTest::addColumn<double>("x");
    QTest::addColumn<double>("sqrLinearTol");
    QTest::addColumn<bool>("expected");

    QTest::newRow("zero-vector") << 0. << 0. << true;
    QTest::newRow("at-tolerance-boundary") << 0.5 << 0.25 << true;
    QTest::newRow("above-tolerance-boundary") << 0.5001 << 0.25 << false;
}

} // namespace Mayo
