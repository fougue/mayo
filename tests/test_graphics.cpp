/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_graphics.h"

#include "../src/base/application.h"
#include "../src/base/brep_utils.h"
#include "../src/base/document.h"
#include "../src/base/mesh_utils.h"
#include "../src/base/signal.h"
#include "../src/graphics/graphics_scene.h"
#include "../src/graphics/graphics_shape_object_driver.h"
#include "../src/gui/gui_application.h"
#include "../src/gui/gui_document.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRep_Tool.hxx>

#include <QtTest/QtTest>

#include <memory>
#include <vector>

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

void TestGraphics::GuiApplication_documentMappingLifecycle_test()
{
    // A document gets one GUI counterpart that is removed when the document closes
    auto app = makeOccHandle<Application>();
    GuiApplication guiApp(app);

    // Track mapping notifications and retain the GUI document pointers for identity checks
    int addedCount = 0;
    int erasedCount = 0;
    GuiDocument* addedGuiDoc = nullptr;
    GuiDocument* erasedGuiDoc = nullptr;
    ScopedSignalConnections<> conns;
    conns.add(guiApp.signalGuiDocumentAdded.connectSlot([&](GuiDocument* guiDoc) {
        ++addedCount;
        addedGuiDoc = guiDoc;
    }));
    conns.add(guiApp.signalGuiDocumentErased.connectSlot([&](GuiDocument* guiDoc) {
        ++erasedCount;
        erasedGuiDoc = guiDoc;
    }));

    // Creating a base document should create exactly one GUI document
    DocumentPtr doc = app->newDocument();
    QVERIFY(!doc.IsNull());

    QCOMPARE(addedCount, 1);
    QCOMPARE(erasedCount, 0);
    QCOMPARE(guiApp.guiDocuments().size(), size_t(1));
    GuiDocument* guiDoc = guiApp.findGuiDocument(doc);
    QVERIFY(guiDoc);
    QCOMPARE(guiDoc, addedGuiDoc);
    QCOMPARE(guiDoc->document().get(), doc.get());

    // Closing the base document should remove its GUI counterpart
    app->closeDocument(doc);

    QCOMPARE(addedCount, 1);
    QCOMPARE(erasedCount, 1);
    QCOMPARE(erasedGuiDoc, guiDoc);
    QVERIFY(guiApp.guiDocuments().empty());
    QVERIFY(!guiApp.findGuiDocument(doc));
}

void TestGraphics::GuiApplication_documentMappingCanBeDisabled_test()
{
    // Mapping can be disabled without affecting documents mapped after it is re-enabled
    auto app = makeOccHandle<Application>();
    GuiApplication guiApp(app);
    guiApp.setAutomaticDocumentMapping(false);

    // Documents created while automatic mapping is disabled should remain unmapped
    int addedCount = 0;
    int erasedCount = 0;
    ScopedSignalConnections<> conns;
    conns << guiApp.signalGuiDocumentAdded.connectSlot([&](GuiDocument*) { ++addedCount; });
    conns << guiApp.signalGuiDocumentErased.connectSlot([&](GuiDocument*) { ++erasedCount; });

    DocumentPtr unmappedDoc = app->newDocument();
    QVERIFY(!unmappedDoc.IsNull());
    QCOMPARE(addedCount, 0);
    QCOMPARE(guiApp.guiDocuments().size(), size_t(0));
    QVERIFY(!guiApp.findGuiDocument(unmappedDoc));

    // Re-enabling mapping does not retroactively map documents already present
    guiApp.setAutomaticDocumentMapping(true);
    QVERIFY(guiApp.guiDocuments().empty());
    QVERIFY(!guiApp.findGuiDocument(unmappedDoc));

    DocumentPtr mappedDoc = app->newDocument();
    QVERIFY(!mappedDoc.IsNull());
    QCOMPARE(addedCount, 1);
    QCOMPARE(guiApp.guiDocuments().size(), size_t(1));
    QVERIFY(guiApp.findGuiDocument(mappedDoc));

    // Closing an unmapped document should not erase the mapped document
    app->closeDocument(unmappedDoc);
    QCOMPARE(erasedCount, 0);
    QCOMPARE(guiApp.guiDocuments().size(), size_t(1));

    app->closeDocument(mappedDoc);
    QCOMPARE(erasedCount, 1);
    QVERIFY(guiApp.guiDocuments().empty());
}

void TestGraphics::GuiDocument_nodeVisibilityPropagation_test()
{
    // Hiding a component updates its product node and marks the assembly partially visible
    auto app = makeOccHandle<Application>();
    GuiApplication guiApp(app);
    guiApp.setAutomaticDocumentMapping(false);

    DocumentPtr doc = app->newDocument();
    TopoDS_Compound compound = BRepUtils::makeEmptyCompound();
    BRepUtils::addShape(&compound, BRepPrimAPI_MakeBox(10, 10, 10));
    BRepUtils::addShape(&compound, BRepPrimAPI_MakeBox(5, 5, 5));
    // Use an assembly so visibility changes exercise both component and product nodes
    const TDF_Label assemblyLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(assemblyLabel, compound);
    doc->addEntityTreeNode(assemblyLabel);

    auto guiDoc = std::make_unique<GuiDocument>(doc, &guiApp);
    const Tree<TDF_Label>& modelTree = doc->modelTree();
    const TreeNodeId rootId = doc->firstEntityNodeId();
    std::vector<TreeNodeId> children;
    visitDirectChildren(rootId, modelTree, [&](TreeNodeId id) { children.push_back(id); });
    QCOMPARE(children.size(), size_t(2));
    const TreeNodeId firstChildProductId = modelTree.nodeChildFirst(children.front());
    const TreeNodeId secondChildProductId = modelTree.nodeChildFirst(children.back());
    QVERIFY(firstChildProductId != 0);
    QVERIFY(secondChildProductId != 0);

    // Capture each visibility notification to check its changed-node set
    int changedCount = 0;
    GuiDocument::MapVisibilityByTreeNodeId lastChanged;
    ScopedSignalConnections<> conns;
    conns << guiDoc->signalNodesVisibilityChanged.connectSlot(
        [&](const GuiDocument::MapVisibilityByTreeNodeId& changed) {
            ++changedCount;
            lastChanged = changed;
        }
    );

    // Hiding one component should hide its product and make the assembly partially visible
    guiDoc->setNodeVisible(children.front(), false);
    QCOMPARE(changedCount, 1);
    QVERIFY(guiDoc->nodeVisibleState(children.front()) == CheckState::Off);
    QVERIFY(guiDoc->nodeVisibleState(firstChildProductId) == CheckState::Off);
    QVERIFY(guiDoc->nodeVisibleState(children.back()) == CheckState::On);
    QVERIFY(guiDoc->nodeVisibleState(rootId) == CheckState::Partially);
    QCOMPARE(lastChanged.size(), size_t(3));
    QVERIFY(lastChanged.at(children.front()) == CheckState::Off);
    QVERIFY(lastChanged.at(firstChildProductId) == CheckState::Off);
    QVERIFY(lastChanged.at(rootId) == CheckState::Partially);

    // Reapplying the same state should not emit a redundant notification
    guiDoc->setNodeVisible(children.front(), false);
    QCOMPARE(changedCount, 1);

    // Hiding the assembly updates every node in its subtree
    guiDoc->setNodeVisible(rootId, false);
    QCOMPARE(changedCount, 2);
    traverseTree(rootId, modelTree, [&](TreeNodeId id) {
        QCOMPARE(guiDoc->nodeVisibleState(id), CheckState::Off);
    });
    QCOMPARE(lastChanged.size(), size_t(3));

    // Showing the assembly restores every node in its subtree
    guiDoc->setNodeVisible(rootId, true);
    QCOMPARE(changedCount, 3);
    traverseTree(rootId, modelTree, [&](TreeNodeId id) {
        QCOMPARE(guiDoc->nodeVisibleState(id), CheckState::On);
    });
    QCOMPARE(lastChanged.size(), size_t(5));

    app->closeDocument(doc);
}

void TestGraphics::GuiDocument_entityMappingLifecycle_test()
{
    // Adding and destroying an entity keeps its graphics mapping and bounding box in sync
    auto app = makeOccHandle<Application>();
    GuiApplication guiApp(app);
    guiApp.setAutomaticDocumentMapping(false);
    guiApp.addGraphicsObjectDriver(makeOccHandle<GraphicsShapeObjectDriver>());

    // Create the GUI document before adding an entity to exercise the document signal path
    DocumentPtr doc = app->newDocument();
    auto guiDoc = std::make_unique<GuiDocument>(doc, &guiApp);

    // Track bounding-box notifications emitted when the entity is added and removed
    int boundingBoxChangedCount = 0;
    std::vector<bool> boundingBoxIsVoid;
    ScopedSignalConnections<> conns;
    conns << guiDoc->signalGraphicsBoundingBoxChanged.connectSlot([&](const Bnd_Box& box) {
        ++boundingBoxChangedCount;
        boundingBoxIsVoid.push_back(box.IsVoid());
    });

    // Adding a shape should create and register one graphics object
    const TDF_Label shapeLabel = doc->newEntityShapeLabel();
    doc->xcaf().setShape(shapeLabel, BRepPrimAPI_MakeBox(10, 20, 30));
    doc->addEntityTreeNode(shapeLabel);

    const TreeNodeId entityNodeId = doc->firstEntityNodeId();
    QCOMPARE(boundingBoxChangedCount, 1);
    QCOMPARE(boundingBoxIsVoid.size(), size_t(1));
    QVERIFY(!boundingBoxIsVoid.front());
    QVERIFY(!guiDoc->graphicsBoundingBox().IsVoid());

    // The mapped graphics object should resolve back to its entity tree node
    std::vector<GraphicsObjectPtr> graphicsObjects;
    guiDoc->foreachGraphicsObject(entityNodeId, [&](GraphicsObjectPtr object) {
        graphicsObjects.push_back(object);
    });
    QCOMPARE(graphicsObjects.size(), size_t(1));
    const GraphicsObjectPtr graphicsObject = graphicsObjects.front();
    QCOMPARE(guiDoc->nodeFromGraphicsObject(graphicsObject), entityNodeId);

    // Destroying the entity should clear its graphics mapping and cached bounds
    doc->destroyEntity(entityNodeId);

    QCOMPARE(boundingBoxChangedCount, 2);
    QCOMPARE(boundingBoxIsVoid.size(), size_t(2));
    QVERIFY(boundingBoxIsVoid.back());
    QVERIFY(guiDoc->graphicsBoundingBox().IsVoid());

    graphicsObjects.clear();
    guiDoc->foreachGraphicsObject(entityNodeId, [&](GraphicsObjectPtr object) {
        graphicsObjects.push_back(object);
    });
    QVERIFY(graphicsObjects.empty());
    QCOMPARE(guiDoc->nodeFromGraphicsObject(graphicsObject), TreeNodeId{0});

    app->closeDocument(doc);
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
