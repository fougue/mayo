/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/brep_utils.h"
#include "../src/base/mesh_utils.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>

namespace Mayo {

void TestBase::BRepUtils_makeEmptyCompound_test()
{
    const TopoDS_Compound compound = BRepUtils::makeEmptyCompound();

    QVERIFY(!compound.IsNull());
    QCOMPARE(compound.ShapeType(), TopAbs_COMPOUND);

    TopExp_Explorer explorer(compound, TopAbs_SHAPE);
    QVERIFY(!explorer.More());
}

void TestBase::BRepUtils_addShape_nullArguments_test()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    // Null target pointer: nothing happens
    BRepUtils::addShape(nullptr, box);

    // Null shape: nothing happens
    TopoDS_Shape target;
    BRepUtils::addShape(&target, TopoDS_Shape{});
    QVERIFY(target.IsNull());
}

void TestBase::BRepUtils_addShape_nullTarget_test()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    // A compound is created and the shape is added
    TopoDS_Shape target;
    BRepUtils::addShape(&target, box);

    QVERIFY(!target.IsNull());
    QCOMPARE(target.ShapeType(), TopAbs_COMPOUND);

    int solidCount = 0;
    BRepUtils::forEachSubShape(target, TopAbs_SOLID, [&](const TopoDS_Shape&) { ++solidCount; });
    QCOMPARE(solidCount, 1);
}

void TestBase::BRepUtils_addShape_existingTarget_test()
{
    const TopoDS_Shape box1 = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();
    const TopoDS_Shape box2 = BRepPrimAPI_MakeBox(gp_Pnt(20., 0., 0.), 10., 20., 30.).Shape();

    TopoDS_Shape target;

    // Initialize target with the first shape
    BRepUtils::addShape(&target, box1);

    // Add a second shape to the existing compound
    BRepUtils::addShape(&target, box2);

    QVERIFY(!target.IsNull());
    QCOMPARE(target.ShapeType(), TopAbs_COMPOUND);

    int solidCount = 0;
    BRepUtils::forEachSubShape(target, TopAbs_SOLID, [&](const TopoDS_Shape&) { ++solidCount; });
    QCOMPARE(solidCount, 2);
}

void TestBase::BRepUtils_makeEdge_test()
{
    NCollection_Array1<gp_Pnt> nodes(1, 3);
    nodes(1) = gp_Pnt(0., 0., 0.);
    nodes(2) = gp_Pnt(10., 0., 0.);
    nodes(3) = gp_Pnt(10., 10., 0.);
    auto polygon = makeOccHandle<Poly_Polygon3D>(nodes);

    const TopoDS_Edge edge = BRepUtils::makeEdge(polygon);
    QVERIFY(!edge.IsNull());
    QCOMPARE(edge.ShapeType(), TopAbs_EDGE);

    TopLoc_Location loc;
    const auto resultPolygon = BRep_Tool::Polygon3D(edge, loc);
    QVERIFY(!resultPolygon.IsNull());
    QCOMPARE(resultPolygon, polygon);
}

void TestBase::BRepUtils_makeFace_test()
{
    auto mesh = makeOccHandle<Poly_Triangulation>(3, 1, false/*!hasUVNodes*/);
    MeshUtils::setNode(mesh, 1, gp::Origin());
    MeshUtils::setNode(mesh, 2, gp_Pnt{10., 0., 0.});
    MeshUtils::setNode(mesh, 3, gp_Pnt{0., 10., 0.});
    MeshUtils::setTriangle(mesh, 1, Poly_Triangle{1, 2, 3});

    const TopoDS_Face face = BRepUtils::makeFace(mesh);

    QVERIFY(!face.IsNull());
    QCOMPARE(face.ShapeType(), TopAbs_FACE);

    TopLoc_Location loc;
    const auto resultMesh = BRep_Tool::Triangulation(face, loc);
    QVERIFY(!resultMesh.IsNull());
    QCOMPARE(resultMesh, mesh);
}

void TestBase::BRepUtils_moreComplex_test()
{
    QVERIFY(BRepUtils::moreComplex(TopAbs_COMPOUND, TopAbs_SOLID));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SOLID, TopAbs_SHELL));
    QVERIFY(BRepUtils::moreComplex(TopAbs_SHELL, TopAbs_FACE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_FACE, TopAbs_EDGE));
    QVERIFY(BRepUtils::moreComplex(TopAbs_EDGE, TopAbs_VERTEX));
}

void TestBase::BRepUtils_hashCode_test()
{
    const TopoDS_Shape shapeNull;
    const TopoDS_Shape shapeBase = BRepPrimAPI_MakeBox(25, 25, 25);
    const TopoDS_Shape shapeCopy = shapeBase;
    const TopoDS_Shape shapeOther = BRepPrimAPI_MakeBox(40, 40, 40);
    QCOMPARE(BRepUtils::hashCode(shapeNull), BRepUtils::hashCode(TopoDS_Shape{}));
    QCOMPARE(BRepUtils::hashCode(shapeBase), BRepUtils::hashCode(shapeCopy));
    QVERIFY(BRepUtils::hashCode(shapeBase) != BRepUtils::hashCode(shapeOther));
}

void TestBase::BRepUtils_shapeStringSerialization_test()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    const std::string serialized = BRepUtils::shapeToString(box);
    const TopoDS_Shape restored = BRepUtils::shapeFromString(serialized);

    QVERIFY(!serialized.empty());
    QVERIFY(!restored.IsNull());

    QCOMPARE(restored.ShapeType(), TopAbs_SOLID);

    int faceCount = 0;
    BRepUtils::forEachSubShape(restored, TopAbs_FACE, [&](const TopoDS_Shape&) { ++faceCount; });
    QCOMPARE(faceCount, 6);
}

void TestBase::BRepUtils_forEachSubShape_test()
{
    // A box has 6 faces
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    std::vector<TopoDS_Shape> faces;
    BRepUtils::forEachSubShape(box, TopAbs_FACE, [&](const TopoDS_Shape& face) {
        faces.push_back(face);
    });

    QCOMPARE(faces.size(), std::size_t(6));

    for (const TopoDS_Shape& face : faces)
        QCOMPARE(face.ShapeType(), TopAbs_FACE);

    // No sub-shape: callback must not be called
    int callbackCount = 0;
    BRepUtils::forEachSubShape(box, TopAbs_SOLID, [&](const TopoDS_Shape&) { ++callbackCount; });
    QCOMPARE(callbackCount, 1);
}

void TestBase::BRepUtils_forEachSubShapeExplorer_test()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    TopExp_Explorer explorer(box, TopAbs_FACE);
    int callbackCount = 0;
    BRepUtils::forEachSubShape(explorer, [&](const TopoDS_Shape& shape) {
        ++callbackCount;
        QCOMPARE(shape.ShapeType(), TopAbs_FACE);
    });

    QCOMPARE(callbackCount, 6);
    QVERIFY(!explorer.More());

    // Empty explorer: while body is never entered
    TopExp_Explorer emptyExplorer(box, TopAbs_VERTEX);

    // Consume all vertices first
    while (emptyExplorer.More())
        emptyExplorer.Next();

    callbackCount = 0;
    BRepUtils::forEachSubShape(emptyExplorer, [&](const TopoDS_Shape&) { ++callbackCount; });
    QCOMPARE(callbackCount, 0);
}

void TestBase::BRepUtils_forEachSubFace_test()
{
    const TopoDS_Shape box = BRepPrimAPI_MakeBox(10., 20., 30.).Shape();

    std::vector<TopoDS_Face> faces;
    BRepUtils::forEachSubFace( box, [&](const TopoDS_Face& face) { faces.push_back(face); });
    QCOMPARE(faces.size(), std::size_t(6));

    for (const TopoDS_Face& face : faces)
        QVERIFY(!face.IsNull());

    // A vertex has no sub-faces
    int callbackCount = 0;
    const TopoDS_Shape vertex = BRepBuilderAPI_MakeVertex(gp_Pnt(0., 0., 0.)).Vertex();
    BRepUtils::forEachSubFace(vertex, [&](const TopoDS_Face&) { ++callbackCount; });
    QCOMPARE(callbackCount, 0);
}

void TestBase::BRepUtils_anySubShape_nullShape_test()
{
    QCOMPARE(BRepUtils::anySubShape({}, TopAbs_EDGE, [](TopoDS_Shape) { return true; }), false);
}

void TestBase::BRepUtils_anySubShape_emptyCompound_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    QCOMPARE(BRepUtils::anySubShape(shape, TopAbs_EDGE, [](TopoDS_Shape) { return true; }), false);
}

void TestBase::BRepUtils_anySubShape_noMatchingShape_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeVertex(gp::Origin()));
    QCOMPARE(BRepUtils::anySubShape(shape, TopAbs_EDGE, [](TopoDS_Shape) { return true; }), false);
}

void TestBase::BRepUtils_anySubShape_singleMatch_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 0, 0)));
    QCOMPARE(BRepUtils::anySubShape(shape, TopAbs_EDGE, [](TopoDS_Shape) { return true; }), true);
}

void TestBase::BRepUtils_anySubShape_multipleShapes_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 0, 0)));
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 1, 0)));
    int count = 0;
    const bool found = BRepUtils::anySubShape(shape, TopAbs_EDGE, [&](TopoDS_Shape) { ++count; return false; });
    QVERIFY(!found);
    QCOMPARE(count, 2);
}

void TestBase::BRepUtils_anySubShape_stopOnFirstMatch_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 0, 0)));
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 1, 0)));
    int count = 0;
    const bool found = BRepUtils::anySubShape(shape, TopAbs_EDGE, [&](TopoDS_Shape) { ++count; return true; });
    QVERIFY(found);
    QCOMPARE(count, 1);
}

void TestBase::BRepUtils_anySubShape_shapeType_test()
{
    auto shape = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeVertex(gp::Origin()));
    BRepUtils::addShape(&shape, BRepBuilderAPI_MakeEdge(gp::Origin(), gp_Pnt(1, 0, 0)));
    QCOMPARE(
        BRepUtils::anySubShape(shape, TopAbs_EDGE, [](TopoDS_Shape shape) { return shape.ShapeType() == TopAbs_EDGE; }),
        true
    );
}

} // namespace Mayo
