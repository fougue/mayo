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
#include "../src/gui/gui_application.h"
#include "../src/gui/gui_document.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRep_Tool.hxx>

#include <QtTest/QtTest>

#include <memory>

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
    auto graphicsShape = graphicsShapeDriver->createObject(doc->firstEntityNodeLabel());
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

void TestGraphics::GuiApplication_destroyDisconnectsApplicationSignals_test()
{
    auto app = makeOccHandle<Application>();
    auto guiApp = std::make_unique<GuiApplication>(app);
    guiApp->setAutomaticDocumentMapping(false);
    guiApp.reset();

    // Emitting this signal after GuiApplication destruction must not call its stale slots
    DocumentPtr doc = app->newDocument();
    QVERIFY(!doc.IsNull());
    app->closeDocument(doc);
}

void TestGraphics::GuiDocument_destroyDisconnectsDocumentSignals_test()
{
    auto app = makeOccHandle<Application>();
    GuiApplication guiApp(app);
    guiApp.setAutomaticDocumentMapping(false);
    DocumentPtr doc = app->newDocument();

    auto guiDoc = std::make_unique<GuiDocument>(doc, &guiApp);
    guiDoc.reset();

    // Adding an entity emits Document::signalEntityAdded; no slot may target the destroyed GuiDocument
    const TDF_Label shapeLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(shapeLabel, BRepPrimAPI_MakeBox(10, 10, 10));
    doc->addEntityTreeNode(shapeLabel);

    app->closeDocument(doc);
}

} // namespace Mayo

QTEST_APPLESS_MAIN(Mayo::TestGraphics)
