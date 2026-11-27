/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

// Avoid MSVC conflicts with M_E, M_LOG2, ...
#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#  define _USE_MATH_DEFINES
#endif

#include "test_base.h"

#include "../src/base/enumeration.h"
#include "../src/base/enumeration_fromenum.h"
#include "../src/base/filepath.h"
#include "../src/base/filepath_conv.h"
#include "../src/base/occ_handle.h"
#include "../src/base/meta_enum.h"
#include "../src/base/quantity.h"
#include "../src/base/string_conv.h"

#include <NCollection_String.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include <QtCore/QtDebug>

#include <cassert>
#include <cmath>
#include <climits>
#include <cstring>
#include <string>

namespace Mayo {

void TestBase::TextId_test()
{
    struct TextIdContext {
        MAYO_DECLARE_TEXT_ID_FUNCTIONS(TestBase::TextIdContext)
    };

    QVERIFY(TextIdContext::textIdContext() == "TestBase::TextIdContext");
    QVERIFY(TextIdContext::textId("foof").trContext == "TestBase::TextIdContext");
    QVERIFY(TextIdContext::textId("bark").key == "bark");
    QVERIFY(TextIdContext::textIdTr("shktu") == "shktu");
}

void TestBase::FilePath_test()
{
    const char strTestPath[] = "../as1-oc-214 - 測試文件.stp";
    const FilePath testPath = std_filesystem::u8path(strTestPath);

    {
        const TCollection_AsciiString ascStrTestPath(strTestPath);
        QCOMPARE(filepathTo<TCollection_AsciiString>(testPath), ascStrTestPath);
    }

    {
        const TCollection_ExtendedString extStrTestPath(strTestPath, true/*multi-byte*/);
        QCOMPARE(filepathTo<TCollection_ExtendedString>(testPath), extStrTestPath);
    }
}

void TestBase::OccHandle_test()
{
    {
        struct OccHandleTestClass_0 : public Standard_Transient {
            explicit OccHandleTestClass_0() = default;
        };

        auto hnd = makeOccHandle<OccHandleTestClass_0>();
        QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_0>));
        QVERIFY(hnd.get() != nullptr);
    }

    {
        struct OccHandleTestClass_1 : public Standard_Transient {
            explicit OccHandleTestClass_1() = default;
            explicit OccHandleTestClass_1(const std::string& str) : m_str(str) {}
            std::string m_str;
        };

        {
            auto hnd = makeOccHandle<OccHandleTestClass_1>();
            QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_1>));
            QVERIFY(hnd.get() != nullptr);
            QCOMPARE(hnd->m_str, std::string{});
        }

        {
            auto hnd = makeOccHandle<OccHandleTestClass_1>("Test string value");
            QCOMPARE(typeid(hnd), typeid(OccHandle<OccHandleTestClass_1>));
            QVERIFY(hnd.get() != nullptr);
            QCOMPARE(hnd->m_str, "Test string value");
        }
    }
}

void TestBase::Enumeration_test()
{
    enum class TestBase_Enum1 { Value0, Value1, Value2, Value3, Value4 };
    using TestEnumType = TestBase_Enum1;

    Enumeration baseEnum = Enumeration::fromType<TestEnumType>();
    QVERIFY(!baseEnum.empty());
    QCOMPARE(baseEnum.size(), MetaEnum::count<TestEnumType>());
    QCOMPARE(baseEnum.items().size(), static_cast<unsigned>(baseEnum.size()));
    for (const auto& enumEntry : MetaEnum::entries<TestEnumType>()) {
        QVERIFY(baseEnum.contains(enumEntry.second));
        QCOMPARE(baseEnum.findValueByName(enumEntry.second), int(enumEntry.first));
        QCOMPARE(baseEnum.findItemByName(enumEntry.second)->value, int(enumEntry.first));
        QCOMPARE(baseEnum.findItemByName(enumEntry.second)->name.key, enumEntry.second);
        QCOMPARE(baseEnum.findItemByValue(enumEntry.first), baseEnum.findItemByName(enumEntry.second));
        QCOMPARE(baseEnum.findNameByValue(enumEntry.first), enumEntry.second);
        QCOMPARE(baseEnum.itemAt(baseEnum.findIndexByValue(enumEntry.first)).value, int(enumEntry.first));
        QCOMPARE(baseEnum.itemAt(baseEnum.findIndexByValue(enumEntry.first)).name.key, enumEntry.second);
    }

    baseEnum.chopPrefix("Value");
    for (const Enumeration::Item& item : baseEnum.items()) {
        const int index = std::atoi(item.name.key.data());
        QCOMPARE(&baseEnum.itemAt(index), &item);
    }

    baseEnum.changeTrContext("newTrContext");
    for (const Enumeration::Item& item : baseEnum.items()) {
        QCOMPARE(item.name.trContext, "newTrContext");
    }
}

void TestBase::MetaEnum_test()
{
    QCOMPARE(MetaEnum::name(TopAbs_VERTEX), "TopAbs_VERTEX");
    QCOMPARE(MetaEnum::name(TopAbs_EDGE), "TopAbs_EDGE");
    QCOMPARE(MetaEnum::name(TopAbs_WIRE), "TopAbs_WIRE");
    QCOMPARE(MetaEnum::name(TopAbs_FACE), "TopAbs_FACE");
    QCOMPARE(MetaEnum::name(TopAbs_SHELL), "TopAbs_SHELL");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_SOLID, "TopAbs_"), "SOLID");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_COMPOUND, "TopAbs_"), "COMPOUND");

    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_VERTEX, "Abs"), "TopAbs_VERTEX");
    QCOMPARE(MetaEnum::nameWithoutPrefix(TopAbs_VERTEX, ""), "TopAbs_VERTEX");
}

void TestBase::Quantity_test()
{
    const QuantityArea area = (10 * Quantity_Millimeter) * (5 * Quantity_Centimeter);
    QCOMPARE(area.value(), 500.);
    QCOMPARE((Quantity_Millimeter / 5.).value(), 1/5.);
}

} // namespace Mayo

QTEST_APPLESS_MAIN(Mayo::TestBase)
