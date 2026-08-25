/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/property_builtins.h"
#include "../src/base/property_enumeration.h"
#include "../src/base/property_value_conversion.h"

Q_DECLARE_METATYPE(Mayo::PropertyValueConversion::Variant)
Q_DECLARE_METATYPE(std::shared_ptr<Mayo::Property>)
Q_DECLARE_METATYPE(std::string)

namespace Mayo {

namespace {

template<typename T>
std::shared_ptr<Property> createPropertyPtr()
{
    return std::make_shared<T>(nullptr, TextId{});
}

} // namespace

void TestBase::PropertyValueConversionVariant_toInt_test()
{
    using Variant = PropertyValueConversion::Variant;
    QFETCH(Variant, variant);
    QFETCH(int, toInt);
    QFETCH(bool, ok);

    bool okActual = false;
    QCOMPARE(variant.toInt(&okActual), toInt);
    QCOMPARE(okActual, ok);
}

void TestBase::PropertyValueConversionVariant_toInt_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<Variant>("variant");
    QTest::addColumn<int>("toInt");
    QTest::addColumn<bool>("ok");

    QTest::newRow("false") << Variant{false} << 0 << false;
    QTest::newRow("true") << Variant{true} << 0 << false;
    QTest::newRow("50.25") << Variant{50.25} << int(std::floor(50.25)) << true;
    QTest::newRow("-50.25") << Variant{-50.25} << int(std::floor(-50.25)) << true;
    QTest::newRow("INT_MAX+1") << Variant{double(INT_MAX) + 1.} << 0 << false;
    QTest::newRow("INT_MIN-1") << Variant{double(INT_MIN) - 1.} << 0 << false;
    QTest::newRow("INT_MAX") << Variant{double(INT_MAX)} << INT_MAX << true;
    QTest::newRow("INT_MIN") << Variant{double(INT_MIN)} << INT_MIN << true;
    QTest::newRow("'58'") << Variant{"58"} << 58 << true;
    QTest::newRow("'4.57'") << Variant{"4.57"} << int(std::floor(4.57)) << true;
    QTest::newRow("'non_int_str'") << Variant{"non_int_str"} << 0 << false;

    const uint8_t bytes[] = { 52, 55 }; // ascii: {'4', '7'}
    QTest::newRow("bytes") << Variant{gsl::span<const uint8_t>(bytes)} << 47 << true;
}

void TestBase::PropertyValueConversionVariant_toString_test()
{
    QFETCH(PropertyValueConversion::Variant, variant);
    QFETCH(std::string, toString);

    bool ok = false;
    if (std::holds_alternative<double>(variant)) {
        const std::string str = variant.toString(&ok);
        QCOMPARE(std::stod(str), std::stod(toString));
    }
    else {
        QCOMPARE(variant.toString(&ok), toString);
    }

    QVERIFY(ok);
}

void TestBase::PropertyValueConversionVariant_toString_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<Variant>("variant");
    QTest::addColumn<std::string>("toString");

    QTest::newRow("false") << Variant{false} << std::string{"false"};
    QTest::newRow("true") << Variant{true} << std::string{"true"};
    QTest::newRow("57") << Variant{57} << std::string{"57"};
    QTest::newRow("4.57f") << Variant{4.57f} << std::string{"4.57"};
    QTest::newRow("1.25") << Variant{1.25} << std::string{"1.25"};
    QTest::newRow("'some string'") << Variant{"some string"} << std::string{"some string"};

    const uint8_t bytes[] = { 48, 65 }; // ascii: {'0', 'A'}
    QTest::newRow("bytes") << Variant{gsl::span<const uint8_t>(bytes)} << std::string{"0A"};
}

void TestBase::PropertyValueConversion_test()
{
    QFETCH(std::shared_ptr<Property>, ptrProperty);
    QFETCH(PropertyValueConversion::Variant, variantValue);

    QVERIFY(ptrProperty);

    PropertyValueConversion conv;
    QVERIFY(conv.fromVariant(ptrProperty.get(), variantValue));
    QCOMPARE(conv.toVariant(*ptrProperty.get()), variantValue);
}

void TestBase::PropertyValueConversion_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<std::shared_ptr<Property>>("ptrProperty");
    QTest::addColumn<Variant>("variantValue");
    QTest::newRow("bool(false)") << createPropertyPtr<PropertyBool>() << Variant(false);
    QTest::newRow("bool(true)") << createPropertyPtr<PropertyBool>() << Variant(true);
    QTest::newRow("int(-50)") << createPropertyPtr<PropertyInt>() << Variant(-50);
    QTest::newRow("int(1979)") << createPropertyPtr<PropertyInt>() << Variant(1979);
    QTest::newRow("double(-1e6)") << createPropertyPtr<PropertyDouble>() << Variant(-1e6);
    QTest::newRow("double(3.1415926535)") << createPropertyPtr<PropertyDouble>() << Variant(3.1415926535);
    QTest::newRow("String(\"test\")") << createPropertyPtr<PropertyString>() << Variant("test");
    QTest::newRow("OccPnt(1.15, -0.5, 3.14)") << createPropertyPtr<PropertyOccPnt>() << Variant("1.15, -0.5, 3.14");
    QTest::newRow("OccVec(-1.7, 0.05, 85.1)") << createPropertyPtr<PropertyOccVec>() << Variant("-1.7, 0.05, 85.1");
    QTest::newRow("OccColor(#0000AA)") << createPropertyPtr<PropertyOccColor>() << Variant("#0000AA");
    QTest::newRow("OccColor(#FFFFFF)") << createPropertyPtr<PropertyOccColor>() << Variant("#FFFFFF");
    QTest::newRow("OccColor(#BB0000)") << createPropertyPtr<PropertyOccColor>() << Variant("#BB0000");

    enum class MayoTest_Color { Bleu, Blanc, Rouge };
    QTest::newRow("Enumeration(Color)") << createPropertyPtr<PropertyEnum<MayoTest_Color>>() << Variant("Blanc");

    QTest::newRow("QuantityLength(15mm)") << createPropertyPtr<PropertyLength>() << Variant("15mm");
}

void TestBase::PropertyValueConversion_bugGitHub219_test()
{
    const std::string strPath = "c:\\é_à_À_œ_ç";
    PropertyValueConversion conv;
    PropertyFilePath propFilePath(nullptr, {});
    const bool ok = conv.fromVariant(&propFilePath, strPath);
    QVERIFY(ok);
    //qDebug() << "strPath:" << QByteArray::fromStdString(strPath);
    //qDebug() << "propFilePath:" << QByteArray::fromStdString(propFilePath.value().u8string());
    QCOMPARE(propFilePath.value().u8string(), strPath);
}

void TestBase::PropertyQuantityValueConversion_test()
{
    QFETCH(std::shared_ptr<Property>, ptrProperty);
    QFETCH(PropertyValueConversion::Variant, variantFrom);
    QFETCH(PropertyValueConversion::Variant, variantTo);

    QVERIFY(ptrProperty);

    PropertyValueConversion conv;
    conv.setDoubleToStringPrecision(7);
    QVERIFY(conv.fromVariant(ptrProperty.get(), variantFrom));
    QCOMPARE(conv.toVariant(*ptrProperty.get()), variantTo);
}

void TestBase::PropertyQuantityValueConversion_test_data()
{
    using Variant = PropertyValueConversion::Variant;
    QTest::addColumn<std::shared_ptr<Property>>("ptrProperty");
    QTest::addColumn<Variant>("variantFrom");
    QTest::addColumn<Variant>("variantTo");
    QTest::newRow("Length(25mm)") << createPropertyPtr<PropertyLength>() << Variant("25mm") << Variant("25mm");
    QTest::newRow("Length(2m)") << createPropertyPtr<PropertyLength>() << Variant("2m") << Variant("2000mm");
    QTest::newRow("Angle(1.57079rad)") << createPropertyPtr<PropertyAngle>() << Variant("1.57079rad") << Variant("1.57079rad");
    QTest::newRow("Angle(90°)") << createPropertyPtr<PropertyAngle>() << Variant("90°") << Variant("1.570796rad");
}

} // namespace Mayo
