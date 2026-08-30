/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/tkernel_utils.h"

namespace Mayo {

void TestBase::TKernelUtils_colorToRgb8_test()
{
    QFETCH(double, r);
    QFETCH(double, g);
    QFETCH(double, b);

    QFETCH(int, er);
    QFETCH(int, eg);
    QFETCH(int, eb);

    Quantity_Color c(r, g, b, TKernelUtils::preferredRgbColorType());
    auto rgb = TKernelUtils::colorToRgb8(c);

    QCOMPARE(int(rgb[0]), er);
    QCOMPARE(int(rgb[1]), eg);
    QCOMPARE(int(rgb[2]), eb);
}

void TestBase::TKernelUtils_colorToRgb8_test_data()
{
    QTest::addColumn<double>("r");
    QTest::addColumn<double>("g");
    QTest::addColumn<double>("b");

    QTest::addColumn<int>("er");
    QTest::addColumn<int>("eg");
    QTest::addColumn<int>("eb");

    QTest::newRow("black") << 0.0 << 0.0 << 0.0   << 0   << 0   << 0;
    QTest::newRow("white") << 1.0 << 1.0 << 1.0   << 255 << 255 << 255;
    QTest::newRow("red")   << 1.0 << 0.0 << 0.0   << 255 << 0   << 0;
    QTest::newRow("green") << 0.0 << 1.0 << 0.0   << 0   << 255 << 0;
    QTest::newRow("blue")  << 0.0 << 0.0 << 1.0   << 0   << 0   << 255;
    QTest::newRow("intermediate") << 0.5 << 0.25 << 0.75  << 128 << 64 << 191;
    QTest::newRow("round up")   << 0.501 << 0.0 << 0.0   << 128 << 0 << 0;
    QTest::newRow("round down") << 0.499 << 0.0 << 0.0   << 127 << 0 << 0;
}

void TestBase::TKernelUtils_colorToHex_test()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);
    QFETCH(QString, strHexColor);

    const Quantity_Color color(red / 255., green / 255., blue / 255., Quantity_TOC_RGB);
    const std::string strHexColorActual = TKernelUtils::colorToHex(color);
    QCOMPARE(QString::fromStdString(strHexColorActual), strHexColor);
}

void TestBase::TKernelUtils_colorToHex_test_data()
{
    QTest::addColumn<int>("red");
    QTest::addColumn<int>("green");
    QTest::addColumn<int>("blue");
    QTest::addColumn<QString>("strHexColor");

    QTest::newRow("RGB(  0,  0,  0)") << 0 << 0 << 0 << "#000000";
    QTest::newRow("RGB(255,255,255)") << 255 << 255 << 255 << "#FFFFFF";
    QTest::newRow("RGB(  5,  5,  5)") << 5 << 5 << 5 << "#050505";
    QTest::newRow("RGB(155,208, 67)") << 155 << 208 << 67 << "#9BD043";
    QTest::newRow("RGB(100,150,200)") << 100 << 150 << 200 << "#6496C8";
}

void TestBase::TKernelUtils_colorFromHex_test()
{
    QFETCH(int, red);
    QFETCH(int, green);
    QFETCH(int, blue);
    QFETCH(QString, strHexColor);
    const Quantity_Color expectedColor(red / 255., green / 255., blue / 255., Quantity_TOC_RGB);

    Quantity_Color actualColor;
    QVERIFY(TKernelUtils::colorFromHex(strHexColor.toStdString(), &actualColor));
    QCOMPARE(actualColor, expectedColor);
}

void TestBase::TKernelUtils_colorFromHex_test_data()
{
    QTest::addColumn<int>("red");
    QTest::addColumn<int>("green");
    QTest::addColumn<int>("blue");
    QTest::addColumn<QString>("strHexColor");

    QTest::newRow("RGB(  0,  0,  0)") << 0 << 0 << 0 << "#000000";
    QTest::newRow("RGB(255,255,255)") << 255 << 255 << 255 << "#FFFFFF";
    QTest::newRow("RGB(  5,  5,  5)") << 5 << 5 << 5 << "#050505";
    QTest::newRow("RGB(155,208, 67)") << 155 << 208 << 67 << "#9BD043";
    QTest::newRow("RGB(100,150,200)") << 100 << 150 << 200 << "#6496C8";
}

} // namespace Mayo
