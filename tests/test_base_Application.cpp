/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/caf_utils.h"
#include "../src/base/string_conv.h"
#include "document_fixture.h"
#include "signal_emit_spy.h"

#include <TDataStd_Name.hxx>

#include <algorithm>

namespace Mayo {

namespace {

TDF_Label addNewShapeEntity(const DocumentPtr& doc, const char* name)
{
    const TDF_Label shapeLabel = doc->newEntityShapeLabel();
    TDataStd_Name::Set(shapeLabel, to_OccExtString(name));
    doc->addEntityTreeNode(shapeLabel);
    return shapeLabel;
}

} // namespace

void TestBase::Application_documentLifecycle_test()
{
    auto app = makeOccHandle<Application>();
    QCOMPARE(app->documentCount(), 0);

    SignalEmitSpy spyDocAdded(&app->signalDocumentAdded);
    DocumentPtr doc = app->newDocument();
    QVERIFY(!doc.IsNull());
    QCOMPARE(spyDocAdded.count, 1);
    QCOMPARE(app->documentCount(), 1);

    SignalEmitSpy spyDocClosed(&app->signalDocumentAboutToClose);
    app->closeDocument(doc);
    QCOMPARE(spyDocClosed.count, 1);
    QCOMPARE(app->documentCount(), 0);
}

void TestBase::Application_entityLifecycle_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    QCOMPARE(doc->entityCount(), 0);
    SignalEmitSpy spyEntityAdded(&fixture.app->signalDocumentEntityAdded);
    addNewShapeEntity(doc, "SomeShape");
    QCOMPARE(spyEntityAdded.count, 1);
    QCOMPARE(doc->entityCount(), 1);
    QVERIFY(XCaf::isShape(doc->firstEntityNodeLabel()));
    QCOMPARE(CafUtils::labelAttrStdName(doc->firstEntityNodeLabel()), to_OccExtString("SomeShape"));

    SignalEmitSpy spyEntityDestroyed(&fixture.app->signalDocumentEntityAboutToBeDestroyed);
    doc->destroyEntity(doc->firstEntityNodeId());
    QCOMPARE(spyEntityDestroyed.count, 1);
    QCOMPARE(doc->entityCount(), 0);
}

void TestBase::Application_destroyMultipleEntities_test()
{
    DocumentFixture fixture;
    auto doc = fixture.doc;

    addNewShapeEntity(doc, "Shape1");
    addNewShapeEntity(doc, "Shape2");
    QCOMPARE(doc->entityCount(), 2);

    doc->destroyEntity(doc->firstEntityNodeId());
    QCOMPARE(doc->entityCount(), 1);
    doc->destroyEntity(doc->firstEntityNodeId());
    QCOMPARE(doc->entityCount(), 0);
}

void TestBase::Application_openDocument_test()
{
    auto app = makeOccHandle<Application>();
    Application::defineMayoFormat(app);

    const FilePath filepath = "tests/inputs/cube10.myx";

    QCOMPARE(app->documentCount(), 0);

    PCDM_ReaderStatus readStatus;
    DocumentPtr doc = app->openDocument(filepath, &readStatus);

    QVERIFY(!doc.IsNull());
    QCOMPARE(readStatus, PCDM_RS_OK);
    QCOMPARE(app->documentCount(), 1);

    // The opened document is registered in the application
    QCOMPARE(app->findIndexOfDocument(doc), 0);
    QCOMPARE(app->findDocumentByIdentifier(doc->identifier()), doc);
    QCOMPARE(app->findDocumentByLocation(filepath), doc);

    app->closeDocument(doc);

    QCOMPARE(app->documentCount(), 0);
}

void TestBase::Application_findDocumentByIdentifier_test()
{
    auto app = makeOccHandle<Application>();

    QCOMPARE(app->documentCount(), 0);

    DocumentPtr doc1 = app->newDocument();
    DocumentPtr doc2 = app->newDocument();

    QVERIFY(!doc1.IsNull());
    QVERIFY(!doc2.IsNull());
    QCOMPARE(app->documentCount(), 2);

    // Find existing documents
    QCOMPARE(app->findDocumentByIdentifier(doc1->identifier()), doc1);
    QCOMPARE(app->findDocumentByIdentifier(doc2->identifier()), doc2);

    // Unknown identifier
    const Document::Identifier unknownId = std::max(doc1->identifier(), doc2->identifier()) + 1;
    QVERIFY(app->findDocumentByIdentifier(unknownId).IsNull());

    // A closed document must no longer be found
    const Document::Identifier doc1Id = doc1->identifier();

    app->closeDocument(doc1);

    QCOMPARE(app->documentCount(), 1);
    QVERIFY(app->findDocumentByIdentifier(doc1Id).IsNull());

    // The remaining document is still indexed
    QCOMPARE(app->findDocumentByIdentifier(doc2->identifier()), doc2);

    app->closeDocument(doc2);

    QCOMPARE(app->documentCount(), 0);
}

void TestBase::Application_findDocumentByLocation_test()
{
    auto app = makeOccHandle<Application>();
    Application::defineMayoFormat(app);

    const FilePath filepath1 = "tests/inputs/cube10.myx";
    const FilePath filepath2 = "tests/inputs/cube20.myx";

    // The files must exist and contain valid Mayo documents
    DocumentPtr doc1 = app->openDocument(filepath1);
    DocumentPtr doc2 = app->openDocument(filepath2);

    QVERIFY(!doc1.IsNull());
    QVERIFY(!doc2.IsNull());
    QCOMPARE(app->documentCount(), 2);

    // Find existing documents
    QCOMPARE(app->findDocumentByLocation(filepath1), doc1);
    QCOMPARE(app->findDocumentByLocation(filepath2), doc2);

    // Unknown location
    const FilePath unknownFilepath = "Application_findDocumentByLocation_unknown.myx";

    QVERIFY(app->findDocumentByLocation(unknownFilepath).IsNull());

    // A closed document must no longer be found
    app->closeDocument(doc1);

    QCOMPARE(app->documentCount(), 1);
    QVERIFY(app->findDocumentByLocation(filepath1).IsNull());

    // The remaining document is still indexed
    QCOMPARE(app->findDocumentByLocation(filepath2), doc2);

    app->closeDocument(doc2);

    QCOMPARE(app->documentCount(), 0);
}

void TestBase::Application_findIndexOfDocument_test()
{
    auto app = makeOccHandle<Application>();

    // Document not belonging to the application
    {
        auto otherApp = makeOccHandle<Application>();
        DocumentPtr docFromOtherApp = otherApp->newDocument();
        QCOMPARE(app->findIndexOfDocument(docFromOtherApp), -1);
    }

    // Documents at different indices
    DocumentPtr doc1 = app->newDocument();
    DocumentPtr doc2 = app->newDocument();
    DocumentPtr doc3 = app->newDocument();

    QCOMPARE(app->findIndexOfDocument(doc1), 0);
    QCOMPARE(app->findIndexOfDocument(doc2), 1);
    QCOMPARE(app->findIndexOfDocument(doc3), 2);
}

void TestBase::Application_setAutoExpandCompoundToAssembly_test()
{
    auto app = makeOccHandle<Application>();

    // Default value
    QVERIFY(app->autoExpandCompoundToAssembly());

    // Disable
    app->setAutoExpandCompoundToAssembly(false);
    QVERIFY(!app->autoExpandCompoundToAssembly());

    // Enable again
    app->setAutoExpandCompoundToAssembly(true);
    QVERIFY(app->autoExpandCompoundToAssembly());
}

void TestBase::Application_DocumentIterator_empty_test()
{
    auto app = makeOccHandle<Application>();

    Application::DocumentIterator it(app);

    QVERIFY(!it.hasNext());
    QCOMPARE(it.currentIndex(), -1);
}

void TestBase::Application_DocumentIterator_test()
{
    auto app = makeOccHandle<Application>();

    DocumentPtr doc1 = app->newDocument();
    DocumentPtr doc2 = app->newDocument();
    DocumentPtr doc3 = app->newDocument();

    Application::DocumentIterator it(app);

    // First document
    QVERIFY(it.hasNext());
    QCOMPARE(it.currentIndex(), 0);
    QCOMPARE(it.current(), doc1);

    // Second document
    it.next();
    QVERIFY(it.hasNext());
    QCOMPARE(it.currentIndex(), 1);
    QCOMPARE(it.current(), doc2);

    // Third document
    it.next();
    QVERIFY(it.hasNext());
    QCOMPARE(it.currentIndex(), 2);
    QCOMPARE(it.current(), doc3);

    // End of iteration
    it.next();
    QVERIFY(!it.hasNext());
    QCOMPARE(it.currentIndex(), -1);
}

} // namespace Mayo
