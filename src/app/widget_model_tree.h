/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_item.h"
#include "../gui/gui_document.h"

#include <QtWidgets/QWidget>
#include <functional>
class QItemSelection;
class QTreeWidget;
class QTreeWidgetItem;

#include <memory>

namespace Mayo {

class GuiApplication;
class WidgetModelTreeBuilder;

struct WidgetModelTree_UserActions {
    using FunctionSyncItems = std::function<void()>;
    std::vector<QAction*> items;
    FunctionSyncItems fnSyncItems;
};

class WidgetModelTree : public QWidget {
    Q_OBJECT
public:
    using Item = ApplicationItem;
    using BuilderPtr = std::unique_ptr<WidgetModelTreeBuilder>;

    WidgetModelTree(QWidget* widget = nullptr);
    ~WidgetModelTree();

    void refreshItemText(const ApplicationItem& appItem);

    void registerGuiApplication(GuiApplication* guiApp);

    WidgetModelTree_UserActions createUserActions(QObject* parent);

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
    void onDocumentNameChanged(const DocumentPtr& doc, const std::string& name);
    void onDocumentEntityAdded(const DocumentPtr& doc, TreeNodeId entityId);
    void onDocumentEntityAboutToBeDestroyed(const DocumentPtr& doc, TreeNodeId entityId);

    void onTreeWidgetDocumentSelectionChanged(
        const QItemSelection& selected, const QItemSelection& deselected
    );
    void onApplicationItemSelectionModelChanged(
        Span<const ApplicationItem> selected, Span<const ApplicationItem> deselected
    );

    void connectTreeModelDataChanged(bool on);
    void connectTreeWidgetDocumentSelectionChanged(bool on);

    void onTreeModelDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles
    );
    void onNodesVisibilityChanged(
        const GuiDocument* guiDoc, const std::unordered_map<TreeNodeId, CheckState>& mapNodeId
    );

    QTreeWidgetItem* loadDocumentEntity(const DocumentTreeNode& entityNode);

    QTreeWidgetItem* findTreeItem(const DocumentPtr& doc) const;
    QTreeWidgetItem* findTreeItem(const DocumentTreeNode& node) const;

    WidgetModelTreeBuilder* findSupportBuilder(const DocumentPtr& doc) const;
    WidgetModelTreeBuilder* findSupportBuilder(const DocumentTreeNode& entityNode) const;

    class Ui_WidgetModelTree* m_ui = nullptr;
    GuiApplication* m_guiApp = nullptr;
    std::vector<BuilderPtr> m_vecBuilder;
    QString m_refItemTextTemplate;
    QMetaObject::Connection m_connTreeModelDataChanged;
    QMetaObject::Connection m_connTreeWidgetDocumentSelectionChanged;
};

} // namespace Mayo
