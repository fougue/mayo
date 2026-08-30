/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/brep_utils.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

namespace Mayo {

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
