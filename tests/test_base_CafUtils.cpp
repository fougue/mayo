/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/caf_utils.h"
#include "../src/base/occt_ncollection_harray1_of_builtintypes.h"
#include "../src/base/string_conv.h"

#include <TDF_Data.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_NamedData.hxx>

namespace Mayo {

void TestBase::CafUtils_labelTag_test()
{
    auto data = makeOccHandle<TDF_Data>();
    const TDF_Label rootLabel = data->Root();
    const TDF_Label childLabel = rootLabel.NewChild();

    QCOMPARE(CafUtils::labelTag(rootLabel).ToCString(), "0:");
    QCOMPARE(CafUtils::labelTag(childLabel).ToCString(), "0:1");
}

void TestBase::CafUtils_labelAttrStdName_test()
{
    auto data = makeOccHandle<TDF_Data>();
    const TDF_Label label = data->Root().NewChild();

    QVERIFY(CafUtils::labelAttrStdName(label).Length() == 0);

    TDataStd_Name::Set(label, "Part name");
    QCOMPARE(to_stdString(CafUtils::labelAttrStdName(label)), "Part name");
}

void TestBase::CafUtils_isNullOrEmpty_test()
{
    QVERIFY(CafUtils::isNullOrEmpty(TDF_Label{}));

    auto data = makeOccHandle<TDF_Data>();
    const TDF_Label emptyLabel = data->Root().NewChild();
    QVERIFY(CafUtils::isNullOrEmpty(emptyLabel));

    const TDF_Label labelWithAttribute = emptyLabel.NewChild();
    TDataStd_Name::Set(labelWithAttribute, "Named");
    QVERIFY(!CafUtils::isNullOrEmpty(labelWithAttribute));
}

void TestBase::CafUtils_getNamedDataKeys_test()
{
    QVERIFY(CafUtils::getNamedDataKeys({}).empty());
    QCOMPARE(CafUtils::namedDataCount({}), 0);

    auto data = makeOccHandle<TDataStd_NamedData>();
    QVERIFY(CafUtils::getNamedDataKeys(data).empty());
    QCOMPARE(CafUtils::namedDataCount(data), 0);

    data->SetInteger(L"int1", 496);
    data->SetInteger(L"int2", 987);
    data->SetReal(L"double1", 1.214);
    data->SetReal(L"double2", 15.06);
    data->SetByte(L"byte1", 31);
    data->SetString(L"string1", "Mayo");
    data->SetString(L"string2", "Fougue");

    {
        const int cintArray1[] = { 1, 9, 2, 8, 3, 7, 4};
        const NCollection_Array1<int> intArray1(cintArray1[0], 1, int(std::size(cintArray1)));
        data->SetArrayOfIntegers(L"intArray1", makeOccHandle<NCollection_HArray1OfInteger>(intArray1));
    }

    {
        const double cdoubleArray1[] = { 1.9, 8.2, 3.7, 6.4, 5.5 };
        const NCollection_Array1<double> doubleArray1(cdoubleArray1[0], 1, int(std::size(cdoubleArray1)));
        data->SetArrayOfReals(L"doubleArray1", makeOccHandle<NCollection_HArray1OfReal>(doubleArray1));
    }

    auto keys = CafUtils::getNamedDataKeys(data);
    QCOMPARE(CafUtils::namedDataCount(data), keys.size());

    auto findKey = [](const TCollection_ExtendedString& label, const auto& keys) {
        auto it = std::find_if(
            keys.cbegin(), keys.cend(), [&](const auto& dkey) { return dkey.label() == label; }
        );
        return it != keys.cend() ? *it : CafUtils::NamedDataKey{};
    };

    QCOMPARE(findKey(L"int1", keys).type, CafUtils::NamedDataType::Int);
    QCOMPARE(findKey(L"int2", keys).type, CafUtils::NamedDataType::Int);
    QCOMPARE(findKey(L"double1", keys).type, CafUtils::NamedDataType::Double);
    QCOMPARE(findKey(L"double2", keys).type, CafUtils::NamedDataType::Double);
    QCOMPARE(findKey(L"byte1", keys).type, CafUtils::NamedDataType::Byte);
    QCOMPARE(findKey(L"string1", keys).type, CafUtils::NamedDataType::String);
    QCOMPARE(findKey(L"string2", keys).type, CafUtils::NamedDataType::String);
    QCOMPARE(findKey(L"intArray1", keys).type, CafUtils::NamedDataType::IntArray);
    QCOMPARE(findKey(L"doubleArray1", keys).type, CafUtils::NamedDataType::DoubleArray);
    QCOMPARE(findKey(L"?", keys).type, CafUtils::NamedDataType::None);
}

} // namespace Mayo
