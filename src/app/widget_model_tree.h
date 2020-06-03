/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_item.h"
#include "../base/property.h"

#include <QtWidgets/QWidget>
class QItemSelection;
class QTreeWidget;
class QTreeWidgetItem;

#include <memory>

namespace Mayo {

class Settings;
class WidgetModelTreeBuilder;

class WidgetModelTree : public QWidget {
    Q_OBJECT
public:
    using Item = ApplicationItem;
    using BuilderPtr = std::unique_ptr<WidgetModelTreeBuilder>;

    WidgetModelTree(QWidget* widget = nullptr);
    ~WidgetModelTree();

    void refreshItemText(const ApplicationItem& appItem);

    void loadConfiguration(const Settings* settings, const QString& keyGroup);
    void saveConfiguration(Settings* settings, const QString& keyGroup);

    std::vector<QAction*> createConfigurationActions(QObject* parent);

    // For builders
    static void addPrototypeBuilder(BuilderPtr builder);

    static DocumentTreeNode documentTreeNode(const QTreeWidgetItem* treeItem);
    static void setDocumentTreeNode(QTreeWidgetItem* treeItem, const DocumentTreeNode& node);
    static void setDocument(QTreeWidgetItem* treeItem, const DocumentPtr& doc);

    static bool holdsDocument(const QTreeWidgetItem* treeItem);
    static bool holdsDocumentTreeNode(const QTreeWidgetItem* treeItem);

private:
    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);
    //void onDocumentPropertyChanged(Document* doc, Property* prop);
    void onDocumentNameChanged(const DocumentPtr& doc, const QString& name);
    void onDocumentEntityAdded(const DocumentPtr& doc, TreeNodeId entityId);
    void onDocumentEntityAboutToBeDestroyed(const DocumentPtr& doc, TreeNodeId entityId);
    //void onDocumentEntityPropertyChanged(DocumentItem* docItem, Property* prop);

    void onTreeWidgetDocumentSelectionChanged(
            const QItemSelection& selected, const QItemSelection& deselected);

    QTreeWidgetItem* loadDocumentEntity(const DocumentTreeNode& entityNode);

    QTreeWidgetItem* findTreeItem(const DocumentPtr& doc) const;
    QTreeWidgetItem* findTreeItem(const DocumentTreeNode& node) const;

    WidgetModelTreeBuilder* findSupportBuilder(const DocumentPtr& doc) const;
    WidgetModelTreeBuilder* findSupportBuilder(const DocumentTreeNode& entityNode) const;

    class Ui_WidgetModelTree* m_ui = nullptr;
    std::vector<BuilderPtr> m_vecBuilder;
    QString m_refItemTextTemplate;
};

} // namespace Mayo
