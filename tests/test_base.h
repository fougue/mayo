/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
    void CppUtils_safeStaticCast_test();

    void TextId_test();

    void FilePath_test();

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

    void IO_probeFormat_test();
    void IO_probeFormat_test_data();
    void IO_probeFormatDirect_test();
    void IO_OccStaticVariablesRollback_test();
    void IO_OccStaticVariablesRollback_test_data();
    void IO_bugGitHub166_test();
    void IO_bugGitHub166_test_data();
    void IO_bugGitHub258_test();

    void DoubleToString_test();
    void StringConv_test();

    void BRepUtils_test();

    void CafUtils_test();

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

    void Span_test();

    void initTestCase();
    void cleanupTestCase();

private:
    IO::System* m_ioSystem = nullptr;
};

} // namespace Mayo
