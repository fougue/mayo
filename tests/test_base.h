/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

namespace IO { class System; }

class TestBase : public QObject {
    Q_OBJECT
private slots:
    void Application_test();
    void DocumentRefCount_test();

    void CppUtils_toggle_test();

    void TextId_test();

    void FilePath_test();

    void MessageCollecter_ignoreSingleMessageType_test();
    void MessageCollecter_ignoreSingleMessageType_test_data();
    void MessageCollecter_only_test();
    void MessageCollecter_only_test_data();

    void OccHandle_test();

    void PropertyValueConversionVariant_toInt_test();
    void PropertyValueConversionVariant_toInt_test_data();

    void PropertyValueConversionVariant_toString_test();
    void PropertyValueConversionVariant_toString_test_data();

    void PropertyValueConversion_test();
    void PropertyValueConversion_test_data();
    void PropertyValueConversion_bugGitHub219_test();

    void PropertyQuantityValueConversion_test();
    void PropertyQuantityValueConversion_test_data();

    void DoubleToString_test();
    void StringConv_test();

    void BRepUtils_test();

    void CafUtils_labelTag_test();
    void CafUtils_getNamedDataKeys_test();

    void MeshUtils_test();
    void MeshUtils_test_data();
    void MeshUtils_orientation_test();
    void MeshUtils_orientation_test_data();

    void Enumeration_test();
    void MetaEnum_test();

    void Quantity_test();

    void TKernelUtils_colorToHex_test();
    void TKernelUtils_colorToHex_test_data();
    void TKernelUtils_colorFromHex_test();
    void TKernelUtils_colorFromHex_test_data();

    void Settings_test();

    void UnitSystem_test();
    void UnitSystem_test_data();

    void LibTask_test();
    void LibTree_test();
    void LibTree_nodeRoot_test();
    void LibTree_removeRoot_test();

    void Span_test();

    void XCaf_userDefinedAttributes_test();
};

} // namespace Mayo
