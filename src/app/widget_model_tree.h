/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_item.h"
#include "../base/property.h"

#include <QtWidgets/QWidget>
class QSettings;
class QItemSelection;
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

class WidgetModelTreeBuilder;

class WidgetModelTree : public QWidget {
    Q_OBJECT
public:
    using Item = ApplicationItem;

    WidgetModelTree(QWidget* widget = nullptr);
    ~WidgetModelTree();

    void refreshItemText(const ApplicationItem& appItem);

    void loadConfiguration(const QSettings* settings, const QString& keyGroup);
    void saveConfiguration(QSettings* settings, const QString& keyGroup);

    std::vector<QAction*> createConfigurationActions(QObject* parent);

    // For builders
    static void addPrototypeBuilder(WidgetModelTreeBuilder* builder);

    static DocumentItemNode documentItemNode(const QTreeWidgetItem* treeItem);
    static void setDocumentItemNode(QTreeWidgetItem* treeItem, const DocumentItemNode& node);

    static bool holdsDocument(const QTreeWidgetItem* treeItem);
    static bool holdsDocumentItem(const QTreeWidgetItem* treeItem);
    static bool holdsDocumentItemNode(const QTreeWidgetItem* treeItem);

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);
    void onDocumentPropertyChanged(Document* doc, Property* prop);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onDocumentItemErased(const DocumentItem* docItem);
    void onDocumentItemPropertyChanged(DocumentItem* docItem, Property* prop);

    void onTreeWidgetDocumentSelectionChanged(
            const QItemSelection& selected, const QItemSelection& deselected);

    QTreeWidgetItem* loadDocumentItem(DocumentItem* docItem);

    QTreeWidgetItem* findTreeItem(const Document* doc) const;
    QTreeWidgetItem* findTreeItem(const DocumentItem* docItem) const;

    WidgetModelTreeBuilder* findSupportBuilder(const Document* doc) const;
    WidgetModelTreeBuilder* findSupportBuilder(const DocumentItem* docItem) const;

    class Ui_WidgetModelTree* m_ui = nullptr;
    std::vector<WidgetModelTreeBuilder*> m_vecBuilder;
    QString m_refItemTextTemplate;
};

} // namespace Mayo
