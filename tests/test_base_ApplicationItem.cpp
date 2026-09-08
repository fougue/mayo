/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/application.h"
#include "../src/base/application_item.h"

namespace Mayo {

void TestBase::ApplicationItem_default_test()
{
    ApplicationItem item;

    QVERIFY(!item.isValid());
    QVERIFY(!item.isDocument());
    QVERIFY(!item.isDocumentTreeNode());
    QVERIFY(item.document().IsNull());
    QCOMPARE(item.documentTreeNode(), DocumentTreeNode::null());
}

void TestBase::ApplicationItem_document_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    ApplicationItem item(doc);

    QVERIFY(item.isValid());
    QVERIFY(item.isDocument());
    QVERIFY(!item.isDocumentTreeNode());
    QCOMPARE(item.document().get(), doc.get());
    QCOMPARE(item.documentTreeNode(), DocumentTreeNode::null());
}

void TestBase::ApplicationItem_documentTreeNode_test()
{
    auto app = makeOccHandle<Application>();
    DocumentPtr doc = app->newDocument();

    DocumentTreeNode node(doc, 42);
    ApplicationItem item(node);

    QVERIFY(item.isValid());
    QVERIFY(!item.isDocument());
    QVERIFY(item.isDocumentTreeNode());

    QCOMPARE(item.document().get(), doc.get());
    QCOMPARE(item.documentTreeNode(), node);
}

void TestBase::ApplicationItem_equality_test()
{
    auto app = makeOccHandle<Application>();

    DocumentPtr doc1 = app->newDocument();
    DocumentPtr doc2 = app->newDocument();

    ApplicationItem document1(doc1);
    ApplicationItem document1Copy(doc1);
    ApplicationItem document2(doc2);

    ApplicationItem node1(DocumentTreeNode(doc1, 1));
    ApplicationItem node1Copy(DocumentTreeNode(doc1, 1));
    ApplicationItem node2(DocumentTreeNode(doc1, 2));
    ApplicationItem nodeFromOtherDocument(DocumentTreeNode(doc2, 1));

    // Documents
    QVERIFY(document1 == document1Copy);
    QVERIFY(!(document1 == document2));

    // DocumentTreeNodes
    QVERIFY(node1 == node1Copy);
    QVERIFY(!(node1 == node2));
    QVERIFY(!(node1 == nodeFromOtherDocument));

    // Document vs DocumentTreeNode
    QVERIFY(!(document1 == node1));

    // Invalid ApplicationItems
    ApplicationItem invalid1;
    ApplicationItem invalid2;

    QVERIFY(invalid1 == invalid2);
    QVERIFY(!(invalid1 == document1));
    QVERIFY(!(invalid1 == node1));
}

} // namespace Mayo
