/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "test_measure.h"

#include "qttest_utils.h"
#include "../src/base/application.h"
#include "../src/base/geom_utils.h"
#include "../src/base/task_progress.h"
#include "../src/base/unit_system.h"
#include "../src/io_occ/io_occ_stl.h"
#include "../src/measure/measure_tool_brep.h"
#include "../qtcommon/qstring_conv.h"

#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomConvert_ApproxCurve.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeEllipse.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

#include <QtCore/QtDebug>
#include <cmath>

namespace Mayo {

namespace {

bool compareCircle(const gp_Circ& lhs, const gp_Circ& rhs, double tolerance = Precision::Confusion())
{
    return lhs.Location().IsEqual(rhs.Location(), tolerance)
            && lhs.Axis().Direction().IsEqual(rhs.Axis().Direction(), tolerance)
            && std::abs(lhs.Radius() - rhs.Radius()) < tolerance;
}

TopoDS_Edge makePolygonEdge(const TColgp_Array1OfPnt& points)
{
    TopoDS_Edge edge;
    BRep_Builder builder;
    builder.MakeEdge(edge, new Poly_Polygon3D(points));
    return edge;
}

} // namespace

void TestMeasure::BRepVertexPosition_test()
{
    const gp_Pnt pnt(154.5, 0.87, -487.64);
    const TopoDS_Vertex vertex = BRepBuilderAPI_MakeVertex(pnt);
    const gp_Pnt pntRes = MeasureToolBRep::brepVertexPosition(vertex);
    QVERIFY(pntRes.IsEqual(pnt, Precision::Confusion()));
}

void TestMeasure::BRepCircle_Regular_test()
{
    const gp_Pnt pntCenter{ 75.5, 0.8, 2548.16 };
    const gp_Dir dirNormal{ 1, 1, 1 };
    const double radius = 58.;
    const GC_MakeCircle makeCircle(gp_Ax2(pntCenter, dirNormal), radius);
    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(makeCircle.Value(), 0, 1.57);
    const MeasureCircle circleRes = MeasureToolBRep::brepCircle(edge);
    QVERIFY(circleRes.pntAnchor.IsEqual(GeomUtils::d0(BRepAdaptor_Curve(edge), 0), Precision::Confusion()));
    QVERIFY(circleRes.isArc);
    QVERIFY(compareCircle(circleRes.value, makeCircle.Value()->Circ()));
}

void TestMeasure::BRepCircle_Ellipse_test()
{
    const gp_Pnt pntCenter{ -57.4, 4487.56, 1.8 };
    const gp_Dir dirNormal{ 1, 0, 1 };
    const double radius = 95.;
    const GC_MakeEllipse makeEllipse(gp_Ax2(pntCenter, dirNormal), radius, radius);
    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(makeEllipse.Value(), 0, 2.27);
    const MeasureCircle circleRes = MeasureToolBRep::brepCircle(edge);
    QVERIFY(circleRes.pntAnchor.IsEqual(GeomUtils::d0(BRepAdaptor_Curve(edge), 0), Precision::Confusion()));
    QVERIFY(circleRes.isArc);
    QVERIFY(compareCircle(circleRes.value, gp_Circ(gp_Ax2(pntCenter, dirNormal), radius)));
}

void TestMeasure::BRepCircle_PseudoCircle_test()
{
    const gp_Pnt pntCenter{ 41.85, 1547.27, 45.89 };
    const gp_Dir dirNormal{ 0.5, 1.25, 0.8 };
    const double radius = 25.48;
    const GC_MakeCircle makeCircle(gp_Ax2(pntCenter, dirNormal), radius);
    GeomConvert_ApproxCurve approxCircle(makeCircle.Value(), Precision::Approximation(), GeomAbs_C1, 2048, 8);
    QVERIFY(approxCircle.IsDone());
    QVERIFY(approxCircle.HasResult());
    const TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(approxCircle.Curve(), 0, 2.98);
    const MeasureCircle circleRes = MeasureToolBRep::brepCircle(edge);
    QVERIFY(circleRes.pntAnchor.IsEqual(GeomUtils::d0(BRepAdaptor_Curve(edge), 0), Precision::Confusion()));
    QVERIFY(circleRes.isArc);
    QVERIFY(compareCircle(circleRes.value, makeCircle.Value()->Circ(), Precision::Approximation()));
}

void TestMeasure::BRepCircle_PolygonEdge_test()
{
    const gp_Pnt pntCenter{ 41.85, 1547.27, 45.89 };
    const double radius = 25.48;
    const int pntCount = 128;
    TColgp_Array1OfPnt points(1, pntCount);
    for (int i = 0; i < pntCount; ++i) {
        const double pi = 3.14159265358979323846;
        const double a = 1.5 * pi * (static_cast<double>(i) / static_cast<double>(pntCount));
        const double x = radius * std::cos(a);
        const double y = radius * std::sin(a);
        points.ChangeValue(i + 1) = gp_Pnt{ pntCenter.X() + x, pntCenter.Y() + y, pntCenter.Z() };
    }

    const TopoDS_Edge edge = makePolygonEdge(points);
    const MeasureCircle circle = MeasureToolBRep::brepCircle(edge);
    const GC_MakeCircle makeGeomCircle(gp_Ax2(pntCenter, gp::DZ()), radius);
    QVERIFY(compareCircle(circle.value, makeGeomCircle.Value()->Circ()));
    QVERIFY(circle.isArc);
}

void TestMeasure::BRepMinDistance_TwoPoints_test()
{
    const gp_Pnt pnt1{ 41.85, 1547.27, 45.89 };
    const gp_Pnt pnt2{ -57.4, 4487.56, 1.8 };
    const TopoDS_Shape shape1 = BRepBuilderAPI_MakeVertex(pnt1);
    const TopoDS_Shape shape2 = BRepBuilderAPI_MakeVertex(pnt2);
    const MeasureDistance minDist = MeasureToolBRep::brepMinDistance(shape1, shape2);
    QVERIFY(minDist.pnt1.IsEqual(pnt1, Precision::Confusion()));
    QVERIFY(minDist.pnt2.IsEqual(pnt2, Precision::Confusion()));
    QCOMPARE(UnitSystem::millimeters(minDist.value).value, pnt1.Distance(pnt2));
}

void TestMeasure::BRepMinDistance_TwoBoxes_test()
{
    const gp_Pnt box1_min{ 5, 5, 5 };
    const gp_Pnt box1_max{ 20, 7, 7 };
    const gp_Pnt box2_min{ 40, 5, 5 };
    const gp_Pnt box2_max{ 55, 7, 7 };
    const TopoDS_Shape shape1 = BRepPrimAPI_MakeBox(box1_min, box1_max);
    const TopoDS_Shape shape2 = BRepPrimAPI_MakeBox(box2_min, box2_max);
    const MeasureDistance minDist = MeasureToolBRep::brepMinDistance(shape1, shape2);
    QCOMPARE(UnitSystem::millimeters(minDist.value).value, std::abs(box1_max.X() - box2_min.X()));
    QCOMPARE(UnitSystem::millimeters(minDist.value).value, minDist.pnt1.Distance(minDist.pnt2));
}

void TestMeasure::BRepMinDistance_TwoConfusedFaces_test()
{
    const TopoDS_Face face1 = BRepBuilderAPI_MakeFace(gp_Pln(gp::XOY()));
    const TopoDS_Face face2 = BRepBuilderAPI_MakeFace(gp_Pln(gp::XOY()));
    try {
        const MeasureDistance minDist = MeasureToolBRep::brepMinDistance(face1, face2);
        QCOMPARE(minDist.value.value(), 0.);
    } catch (const IMeasureError& err) {
        qDebug() << to_QString(err.message());
    }
}

void TestMeasure::BRepAngle_TwoLinesIntersect_test()
{
    const TopoDS_Shape shape1 = BRepBuilderAPI_MakeEdge(gp_Lin(gp::Origin(), gp::DX()));
    const TopoDS_Shape shape2 = BRepBuilderAPI_MakeEdge(gp_Lin(gp::Origin(), gp::DY()));
    const MeasureAngle angle = MeasureToolBRep::brepAngle(shape1, shape2);
    QVERIFY(angle.pntCenter.IsEqual(gp::Origin(), Precision::Confusion()));
    QCOMPARE(angle.value, 90. * Quantity_Degree);
}

void TestMeasure::BRepAngle_TwoLinesParallelError_test()
{
    const TopoDS_Shape shape1 = BRepBuilderAPI_MakeEdge(gp_Lin(gp::Origin(), gp::DX()));
    const TopoDS_Shape shape2 = BRepBuilderAPI_MakeEdge(gp_Lin({ 0, 5, 5 }, gp::DX()));
    MAYO_QVERIFY_THROWS_EXCEPTION(IMeasureError, MeasureToolBRep::brepAngle(shape1, shape2));
}

void TestMeasure::BRepLength_PolygonEdge_test()
{
    TColgp_Array1OfPnt points(1, 5);
    points.ChangeValue(1) = gp::Origin();
    points.ChangeValue(2) = gp_Pnt{0, 10, 0};
    points.ChangeValue(3) = gp_Pnt{0, 10, 5};
    points.ChangeValue(4) = gp_Pnt{7, 10, 5};
    points.ChangeValue(5) = gp_Pnt{7, 12, 5};
    const TopoDS_Edge edge = makePolygonEdge(points);
    const MeasureLength len = MeasureToolBRep::brepLength(edge);
    QCOMPARE(UnitSystem::millimeters(len.value).value, 24.);
}

void TestMeasure::BRepArea_TriangulationFace_test()
{
    auto progress = &TaskProgress::null();
    IO::OccStlReader reader;
    const bool okRead = reader.readFile("tests/inputs/face_trsf_scale_almost_1.stl", progress);
    QVERIFY(okRead);

    auto app = makeOccHandle<Application>();
    auto doc = app->newDocument();
    const TDF_LabelSequence seqLabel = reader.transfer(doc, progress);
    QCOMPARE(seqLabel.Size(), 1);
    const TopoDS_Shape shape = doc->xcaf().shape(seqLabel.First());
    const MeasureArea area = MeasureToolBRep::brepArea(shape);
    QVERIFY(std::abs(double(UnitSystem::squareMillimeters(area.value)) - 597.6224) < 0.0001);
}

void TestMeasure::BRepBoundingBox_Sphere_test()
{
    const double sphereRadius = 50.;
    const TopoDS_Shape sphereShape = BRepPrimAPI_MakeSphere(sphereRadius);
    const MeasureBoundingBox bndBox = MeasureToolBRep::brepBoundingBox(sphereShape);
    QVERIFY(bndBox.cornerMin.IsEqual(gp_Pnt{-sphereRadius, -sphereRadius, -sphereRadius}, Precision::Confusion()));
    QVERIFY(bndBox.cornerMax.IsEqual(gp_Pnt{sphereRadius, sphereRadius, sphereRadius}, Precision::Confusion()));
    QCOMPARE(double(UnitSystem::millimeters(bndBox.xLength)), 2 * sphereRadius);
    QCOMPARE(double(UnitSystem::millimeters(bndBox.yLength)), 2 * sphereRadius);
    QCOMPARE(double(UnitSystem::millimeters(bndBox.zLength)), 2 * sphereRadius);
    QCOMPARE(double(UnitSystem::cubicMillimeters(bndBox.volume)), 8 * sphereRadius * sphereRadius * sphereRadius);
}

void TestMeasure::BRepBoundingBox_NullShape_test()
{
    const TopoDS_Shape nullShape;
    MAYO_QVERIFY_THROWS_EXCEPTION(IMeasureError, MeasureToolBRep::brepBoundingBox(nullShape));
}

} // namespace Mayo
