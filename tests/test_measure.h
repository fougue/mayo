/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

class TestMeasure : public QObject {
    Q_OBJECT
private slots:
    void BRepVertexPosition_test();

    void BRepCircle_Regular_test();
    void BRepCircle_Ellipse_test();
    void BRepCircle_PseudoCircle_test();

    void BRepMinDistance_TwoPoints_test();
    void BRepMinDistance_TwoBoxes_test();

    void BRepAngle_TwoLinesIntersect_test();
    void BRepAngle_TwoLinesParallelError_test();

    void BRepLength_PolygonEdge_test();
};

} // namespace Mayo
