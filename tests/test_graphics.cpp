/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_graphics.h"

#include "../src/base/application.h"
#include "../src/base/brep_utils.h"
#include "../src/base/document.h"
#include "../src/base/mesh_utils.h"
#include "../src/graphics/graphics_scene.h"
#include "../src/graphics/graphics_shape_object_driver.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRep_Tool.hxx>

#include <QtTest/QtTest>

namespace Mayo {

void TestGraphics::Regression_bugGitHub255_test()
{
    // SEE
    //   https://github.com/fougue/mayo/issues/255
    //   https://github.com/fougue/mayo/issues/376

    // OpenCascade by default triangulates the shapes to be displayed, even though they already have
    // precomputed triangulations
    // This test checks that displaying a shape with Mayo graphics doesn't affect the triangulation

    auto app = makeOccHandle<Application>();
    auto doc = app->newDocument();

    const TDF_Label shapeLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(shapeLabel, BRepPrimAPI_MakeBox(25, 25, 25));
    doc->addEntityTreeNode(shapeLabel);
    QCOMPARE(doc->entityCount(), 1);
    QVERIFY(XCaf::isShape(shapeLabel));

    auto graphicsShapeDriver = makeOccHandle<GraphicsShapeObjectDriver>();
    auto graphicsShape = graphicsShapeDriver->createObject(doc->entityLabel(0));
    QVERIFY(!graphicsShape.IsNull());
    QCOMPARE(graphicsShape->GetOwner(), graphicsShapeDriver);

    GraphicsScene graphicsScene;
    graphicsScene.addObject(graphicsShape);

    BRepUtils::forEachSubFace(XCaf::shape(shapeLabel), [&](const TopoDS_Face& face) {
        TopLoc_Location loc;
        auto mesh = BRep_Tool::Triangulation(face, loc);
        QVERIFY(mesh.IsNull() || MeshUtils::triangles(mesh).IsEmpty());
    });
}

} // namespace Mayo