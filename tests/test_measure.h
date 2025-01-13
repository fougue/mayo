/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#  define _USE_MATH_DEFINES // Fix M_E, M_PI, ... macro redefinitions with qmath.h
#endif
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
    void BRepCircle_PolygonEdge_test();

    void BRepMinDistance_TwoPoints_test();
    void BRepMinDistance_TwoBoxes_test();
    void BRepMinDistance_TwoConfusedFaces_test();

    void BRepAngle_TwoLinesIntersect_test();
    void BRepAngle_TwoLinesParallelError_test();

    void BRepLength_PolygonEdge_test();

    void BRepArea_TriangulationFace_test();

    void BRepBoundingBox_Sphere_test();
    void BRepBoundingBox_NullShape_test();
};

} // namespace Mayo
