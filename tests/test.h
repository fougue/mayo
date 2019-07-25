/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
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
    void BRepUtils_test();
    void CafUtils_test();
    void MeshUtils_test();
    void MeshUtils_test_data();
    void Quantity_test();
    void Result_test();
    void StringUtils_text_test();
    void StringUtils_text_test_data();
    void StringUtils_skipWhiteSpaces_test();
    void StringUtils_skipWhiteSpaces_test_data();
    void UnitSystem_test();
    void UnitSystem_test_data();

    void LibTree_test();
};

} // namespace Mayo
