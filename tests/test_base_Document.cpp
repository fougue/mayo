/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "document_fixture.h"
#include "../src/base/caf_utils.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <TDataStd_Name.hxx>

#include <vector>

namespace Mayo {

void TestBase::Document_refCount_test()
{
    auto app = makeOccHandle<Application>();
    auto doc = app->newDocument();
    QVERIFY(doc->GetRefCount() > 1);
    app->closeDocument(doc);
    QCOMPARE(doc->GetRefCount(), 1);
}

void TestBase::Document_name_test()
{
    DocumentFixture fixture;
    QVERIFY(fixture.doc->name().empty());
}

void TestBase::Document_setName_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    std::vector<std::string> changedNames;
    doc->signalNameChanged.connectSlot(
        [&](const std::string& name) { changedNames.push_back(name); }
    );

    doc->setName("First name");
    doc->setName("Updated name");

    QVERIFY(doc->name() == "Updated name");
    QCOMPARE(changedNames.size(), size_t{2});
    QVERIFY(changedNames[0] == "First name");
    QVERIFY(changedNames[1] == "Updated name");
}

void TestBase::Document_isEntity_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    const TDF_Label label = doc->newEntityLabel();
    TDataStd_Name::Set(label, "Entity");
    doc->addEntityTreeNode(label);

    const TreeNodeId entityId = doc->firstEntityNodeId();
    QVERIFY(doc->isEntity(entityId));
    QVERIFY(!doc->isEntity(0));
}

void TestBase::Document_rebuildModelTree_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    const TDF_Label firstLabel = doc->newEntityLabel();
    TDataStd_Name::Set(firstLabel, "First");
    doc->addEntityTreeNode(firstLabel);
    QCOMPARE(doc->entityCount(), 1);

    const TDF_Label secondLabel = doc->newEntityLabel();
    TDataStd_Name::Set(secondLabel, "Second");

    // Rebuilding replaces the current tree with all populated top-level labels
    doc->rebuildModelTree();
    QCOMPARE(doc->entityCount(), 2);
    doc->rebuildModelTree();
    QCOMPARE(doc->entityCount(), 2);

    bool foundFirst = false;
    bool foundSecond = false;
    for (TreeNodeId nodeId : doc->allEntityNodeIds()) {
        const TDF_Label nodeLabel = doc->modelTreeNodeLabel(nodeId);
        foundFirst |= nodeLabel == firstLabel;
        foundSecond |= nodeLabel == secondLabel;
    }
    QVERIFY(foundFirst);
    QVERIFY(foundSecond);
}

void TestBase::Document_rebuildModelTree_assembly_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    auto shapeTool = doc->xcaf().shapeTool();
    const TDF_Label assemblyLabel = shapeTool->NewShape();
    TDataStd_Name::Set(assemblyLabel, "Assembly");

    const TopoDS_Shape box = BRepPrimAPI_MakeBox(1., 2., 3.).Shape();
    shapeTool->AddComponent(assemblyLabel, box);
    shapeTool->AddComponent(assemblyLabel, box);

    doc->rebuildModelTree();

    QCOMPARE(doc->entityCount(), 1);
    const Tree<TDF_Label>& tree = doc->modelTree();
    const TreeNodeId assemblyNodeId = doc->firstEntityNodeId();
    QVERIFY(XCaf::isShapeAssembly(tree.nodeData(assemblyNodeId)));

    const TreeNodeId firstComponentId = tree.nodeChildFirst(assemblyNodeId);
    QVERIFY(firstComponentId != 0);
    QVERIFY(XCaf::isShapeComponent(tree.nodeData(firstComponentId)));
    QVERIFY(XCaf::isShapeReference(tree.nodeData(firstComponentId)));

    const TreeNodeId firstReferredShapeId = tree.nodeChildFirst(firstComponentId);
    QVERIFY(firstReferredShapeId != 0);
    QVERIFY(XCaf::isShapeSimple(tree.nodeData(firstReferredShapeId)));

    const TreeNodeId secondComponentId = tree.nodeSiblingNext(firstComponentId);
    QVERIFY(secondComponentId != 0);
    QVERIFY(XCaf::isShapeComponent(tree.nodeData(secondComponentId)));
    QVERIFY(XCaf::isShapeReference(tree.nodeData(secondComponentId)));

    const TreeNodeId secondReferredShapeId = tree.nodeChildFirst(secondComponentId);
    QVERIFY(secondReferredShapeId != 0);
    QVERIFY(XCaf::isShapeSimple(tree.nodeData(secondReferredShapeId)));
    QCOMPARE(tree.nodeSiblingNext(secondComponentId), TreeNodeId{0});
}

void TestBase::Document_newEntityLabel_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    const TDF_Label firstLabel = doc->newEntityLabel();
    const TDF_Label secondLabel = doc->newEntityLabel();

    QVERIFY(!firstLabel.IsNull());
    QVERIFY(!secondLabel.IsNull());
    QVERIFY(firstLabel != secondLabel);
    QVERIFY(firstLabel.Father() == doc->rootLabel());
    QVERIFY(secondLabel.Father() == doc->rootLabel());
    QVERIFY(CafUtils::isNullOrEmpty(firstLabel));
    QVERIFY(CafUtils::isNullOrEmpty(secondLabel));
}

void TestBase::Document_destroyEntity_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    const TDF_Label label = doc->newEntityLabel();
    TDataStd_Name::Set(label, "Entity to destroy");
    doc->addEntityTreeNode(label);
    const TreeNodeId entityId = doc->firstEntityNodeId();

    std::vector<TreeNodeId> aboutToBeDestroyed;
    doc->signalEntityAboutToBeDestroyed.connectSlot(
        [&](TreeNodeId id) { aboutToBeDestroyed.push_back(id); }
    );

    doc->destroyEntity(entityId);

    QCOMPARE(doc->entityCount(), 0);
    QVERIFY(!doc->isEntity(entityId));
    QVERIFY(CafUtils::isNullOrEmpty(label));
    QCOMPARE(aboutToBeDestroyed.size(), size_t(1));
    QCOMPARE(aboutToBeDestroyed.front(), entityId);
}

} // namespace Mayo
