/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/application.h"
#include "../src/base/application_item_selection_model.h"

namespace Mayo {

void TestBase::ApplicationItemSelectionModel_selectedItems_test()
{
    ApplicationItemSelectionModel model;

    QCOMPARE(model.selectedItems().size(), 0);

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item{DocumentTreeNode{doc, 1}};
    QVERIFY(item.isValid());

    model.add(item);

    const auto selectedItems = model.selectedItems();
    QCOMPARE(selectedItems.size(), 1);
    QVERIFY(selectedItems[0] == item);
}

void TestBase::ApplicationItemSelectionModel_isSelected_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};

    QVERIFY(item1.isValid());
    QVERIFY(item2.isValid());
    QVERIFY(!(item1 == item2));

    QVERIFY(!model.isSelected(item1));
    QVERIFY(!model.isSelected(item2));

    model.add(item1);

    QVERIFY(model.isSelected(item1));
    QVERIFY(!model.isSelected(item2));

    model.remove(item1);

    QVERIFY(!model.isSelected(item1));
}

void TestBase::ApplicationItemSelectionModel_addItem_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};

    int signalCount = 0;
    std::vector<ApplicationItem> addedItems;
    std::vector<ApplicationItem> removedItems;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            ++signalCount;
            addedItems.assign(added.begin(), added.end());
            removedItems.assign(removed.begin(), removed.end());
        });

    model.add(item1);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 1);

    QCOMPARE(addedItems.size(), 1);
    QVERIFY(addedItems[0] == item1);
    QVERIFY(removedItems.empty());

    // Adding an already selected item does nothing
    model.add(item1);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 1);
}

void TestBase::ApplicationItemSelectionModel_addItems_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};
    const ApplicationItem item3{DocumentTreeNode{doc, 3}};

    model.add(item1);

    int signalCount = 0;
    std::vector<ApplicationItem> addedItems;
    std::vector<ApplicationItem> removedItems;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            ++signalCount;
            addedItems.assign(added.begin(), added.end());
            removedItems.assign(removed.begin(), removed.end());
        });

    // item1 is already selected; item2 and item3 are new
    const ApplicationItem items[] = {item1, item2, item3};
    model.add(items);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 3);

    QCOMPARE(addedItems.size(), 2);
    QVERIFY(addedItems[0] == item2);
    QVERIFY(addedItems[1] == item3);
    QVERIFY(removedItems.empty());

    // All items are already selected
    model.add(items);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 3);

    // Empty span
    model.add(gsl::span<ApplicationItem>{});

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 3);
}

void TestBase::ApplicationItemSelectionModel_removeItem_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};

    model.add(item1);

    int signalCount = 0;
    std::vector<ApplicationItem> addedItems;
    std::vector<ApplicationItem> removedItems;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            ++signalCount;
            addedItems.assign(added.begin(), added.end());
            removedItems.assign(removed.begin(), removed.end());
        });

    // Remove a selected item
    model.remove(item1);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);

    QVERIFY(addedItems.empty());
    QCOMPARE(removedItems.size(), 1);
    QVERIFY(removedItems[0] == item1);

    // Remove an item which is not selected
    model.remove(item2);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);
}

void TestBase::ApplicationItemSelectionModel_removeItems_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};
    const ApplicationItem item3{DocumentTreeNode{doc, 3}};

    const ApplicationItem selectedItems[] = {item1, item3};
    model.add(selectedItems);

    int signalCount = 0;
    std::vector<ApplicationItem> addedItems;
    std::vector<ApplicationItem> removedItems;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            ++signalCount;
            addedItems.assign(added.begin(), added.end());
            removedItems.assign(removed.begin(), removed.end());
        });

    // item1 and item3 are selected; item2 is not
    const ApplicationItem items[] = {item1, item2, item3};
    model.remove(items);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);

    QVERIFY(addedItems.empty());
    QCOMPARE(removedItems.size(), 2);
    QVERIFY(removedItems[0] == item1);
    QVERIFY(removedItems[1] == item3);

    // Nothing is selected anymore
    model.remove(items);

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);

    // Empty span
    model.remove(gsl::span<ApplicationItem>{});

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);
}

void TestBase::ApplicationItemSelectionModel_clear_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};

    // Clearing an empty selection does nothing
    model.clear();

    QCOMPARE(model.selectedItems().size(), 0);

    const ApplicationItem items[] = {item1, item2};
    model.add(items);

    QCOMPARE(model.selectedItems().size(), 2);

    int signalCount = 0;
    std::vector<ApplicationItem> removedItems;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            ++signalCount;
            QVERIFY(added.empty());
            removedItems.assign(removed.begin(), removed.end());
        });

    model.clear();

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);

    QCOMPARE(removedItems.size(), 2);
    QVERIFY(removedItems[0] == item1);
    QVERIFY(removedItems[1] == item2);

    // Clearing an already empty selection does nothing
    model.clear();

    QCOMPARE(signalCount, 1);
    QCOMPARE(model.selectedItems().size(), 0);
}

void TestBase::ApplicationItemSelectionModel_clearSignal_test()
{
    ApplicationItemSelectionModel model;

    auto app = makeOccHandle<Application>();
    const DocumentPtr doc = app->newDocument();

    const ApplicationItem item1{DocumentTreeNode{doc, 1}};
    const ApplicationItem item2{DocumentTreeNode{doc, 2}};

    const ApplicationItem items[] = {item1, item2};
    model.add(items);

    bool selectionWasClearedWhenSignalWasEmitted = false;

    model.signalChanged.connectSlot(
        [&](gsl::span<const ApplicationItem> added, gsl::span<const ApplicationItem> removed) {
            QVERIFY(added.empty());

            QCOMPARE(removed.size(), 2);
            QVERIFY(removed[0] == item1);
            QVERIFY(removed[1] == item2);

            // clear() must clear the model before emitting the signal
            selectionWasClearedWhenSignalWasEmitted = model.selectedItems().empty();
        });

    model.clear();

    QVERIFY(selectionWasClearedWhenSignalWasEmitted);
    QVERIFY(model.selectedItems().empty());
}

} // namespace Mayo
