/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

class Test : public QObject {
    Q_OBJECT
private slots:
    void Application_test();

    void TextId_test();

    void FilePath_test();

    void PropertyValueConversion_test();
    void PropertyValueConversion_test_data();
    void PropertyQuantityValueConversion_test();
    void PropertyQuantityValueConversion_test_data();

    void IO_test();
    void IO_test_data();
    void IO_OccStaticVariablesRollback_test();
    void IO_OccStaticVariablesRollback_test_data();

    void BRepUtils_test();

    void CafUtils_test();

    void MeshUtils_test();
    void MeshUtils_test_data();
    void MeshUtils_orientation_test();
    void MeshUtils_orientation_test_data();

    void MetaEnum_test();

    void Quantity_test();

    void Result_test();

    void QStringUtils_append_test();
    void QStringUtils_append_test_data();
    void QStringUtils_text_test();
    void QStringUtils_text_test_data();

    void StringConv_test();

    void TKernelUtils_colorToHex_test();
    void TKernelUtils_colorToHex_test_data();
    void TKernelUtils_colorFromHex_test();
    void TKernelUtils_colorFromHex_test_data();

    void UnitSystem_test();
    void UnitSystem_test_data();

    void LibTask_test();
    void LibTree_test();

    void QtGuiUtils_test();

    void initTestCase();
};

} // namespace Mayo
