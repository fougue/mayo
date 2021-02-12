/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree.h"

#include "../base/application.h"
#include "../base/application_item_selection_model.h"
#include "../base/document.h"
#include "../base/settings.h"
#include "../base/string_utils.h"
#include "../gui/gui_application.h"
#include "item_view_buttons.h"
#include "theme.h"
#include "widget_model_tree_builder.h"

#include <QtCore/QtDebug>
#include <QtCore/QMetaType>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <gsl/gsl_util>
#include <cassert>
#include <memory>
#include <unordered_map>

Q_DECLARE_METATYPE(Mayo::DocumentPtr)
Q_DECLARE_METATYPE(Mayo::DocumentTreeNode)

namespace Mayo {
namespace Internal {

static std::vector<WidgetModelTree::BuilderPtr>& arrayPrototypeBuilder()
{
    static std::vector<WidgetModelTree::BuilderPtr> vecPtrBuilder;
    if (vecPtrBuilder.empty())
        vecPtrBuilder.emplace_back(std::make_unique<WidgetModelTreeBuilder>()); // Fallback

    return vecPtrBuilder;
}

template<typename PREDICATE>
static WidgetModelTreeBuilder* findSupportBuilder(
        Span<WidgetModelTreeBuilder*> spanBuilder, PREDICATE fn)
{
    for (int i = 1; i < spanBuilder.size(); ++i) {
        if (fn(spanBuilder.at(i)))
            return spanBuilder.at(i);
    }

    return spanBuilder.at(0); // Fallback
}

class TreeWidget : public QTreeWidget {
public:
    TreeWidget(QWidget* parent = nullptr)
        : QTreeWidget(parent)
    { }

    QModelIndex indexFromItem(QTreeWidgetItem* item, int column = 0) const {
        return QTreeWidget::indexFromItem(item, column);
    }

    QTreeWidgetItem* itemFromIndex(const QModelIndex& index) const {
        return QTreeWidget::itemFromIndex(index);
    }
};

} // namespace Internal
} // namespace Mayo
#include "ui_widget_model_tree.h"

namespace Mayo {

namespace Internal {

enum TreeItemRole {
    TreeItemTypeRole = Qt::UserRole + 1,
    TreeItemDocumentRole,
    TreeItemDocumentTreeNodeRole
};

enum TreeItemType {
    TreeItemType_Unknown = 0,
    TreeItemType_Document = 0x01,
    TreeItemType_DocumentTreeNode = 0x02,
    TreeItemType_DocumentEntity = 0x10 | TreeItemType_DocumentTreeNode
};

static TreeItemType treeItemType(const QTreeWidgetItem* treeItem)
{
    const QVariant varType = treeItem->data(0, TreeItemTypeRole);
    return varType.isValid() ?
                static_cast<Internal::TreeItemType>(varType.toInt()) :
                Internal::TreeItemType_Unknown;
}

static DocumentPtr treeItemDocument(const QTreeWidgetItem* treeItem)
{
    //Expects(treeItemType(treeItem) == TreeItemType_Document);
    const QVariant var = treeItem->data(0, TreeItemDocumentRole);
    return var.isValid() ? var.value<DocumentPtr>() : DocumentPtr();
}

static DocumentTreeNode treeItemDocumentTreeNode(const QTreeWidgetItem* treeItem)
{
    //Expects(treeItemType(treeItem) & TreeItemType_DocumentTreeNode);
    const QVariant var = treeItem->data(0, TreeItemDocumentTreeNodeRole);
    return var.isValid() ? var.value<DocumentTreeNode>() : DocumentTreeNode::null();
}

static void setTreeItemDocument(QTreeWidgetItem* treeItem, const DocumentPtr& doc)
{
    treeItem->setData(0, TreeItemTypeRole, Internal::TreeItemType_Document);
    treeItem->setData(0, TreeItemDocumentRole, QVariant::fromValue(doc));
}

static void setTreeItemDocumentTreeNode(QTreeWidgetItem* treeItem, const DocumentTreeNode& node)
{
    const TreeItemType treeItemType =
            node.isEntity() ? TreeItemType_DocumentEntity : TreeItemType_DocumentTreeNode;
    treeItem->setData(0, TreeItemTypeRole, treeItemType);
    treeItem->setData(0, TreeItemDocumentTreeNodeRole, QVariant::fromValue(node));
}

static ApplicationItem toApplicationItem(const QTreeWidgetItem* treeItem)
{
    const TreeItemType type = Internal::treeItemType(treeItem);
    if (type == TreeItemType_Document)
        return ApplicationItem(Internal::treeItemDocument(treeItem));
    else if (type & TreeItemType_DocumentTreeNode)
        return ApplicationItem(Internal::treeItemDocumentTreeNode(treeItem));

    return ApplicationItem();
}

} // namespace Internal

WidgetModelTree::WidgetModelTree(QWidget* widget)
    : QWidget(widget),
      m_ui(new Ui_WidgetModelTree)
{
    m_ui->setupUi(this);
    m_ui->treeWidget_Model->setUniformRowHeights(true);
    for (const BuilderPtr& ptrBuilder : Internal::arrayPrototypeBuilder()) {
        m_vecBuilder.push_back(std::move(ptrBuilder->clone()));
        m_vecBuilder.back()->setTreeWidget(m_ui->treeWidget_Model);
    }

    // Add action "Remove item from document"
    auto modelTreeBtns = new ItemViewButtons(m_ui->treeWidget_Model, this);
    constexpr int idBtnRemove = 1;
    modelTreeBtns->addButton(
                idBtnRemove,
                mayoTheme()->icon(Theme::Icon::Cross),
                tr("Remove from document"));
    modelTreeBtns->setButtonDetection(
                idBtnRemove,
                Internal::TreeItemTypeRole,
                QVariant(Internal::TreeItemType_DocumentEntity));
    modelTreeBtns->setButtonDisplayColumn(idBtnRemove, 0);
    modelTreeBtns->setButtonDisplayModes(idBtnRemove, ItemViewButtons::DisplayOnDetection);
    modelTreeBtns->setButtonItemSide(idBtnRemove, ItemViewButtons::ItemRightSide);
    const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    modelTreeBtns->setButtonIconSize(idBtnRemove, QSize(iconSize * 0.66, iconSize * 0.66));
    modelTreeBtns->installDefaultItemDelegate();
    QObject::connect(
                modelTreeBtns, &ItemViewButtons::buttonClicked,
                this, [=](int btnId, const QModelIndex& index) {
        if (btnId == idBtnRemove && index.isValid()) {
            const QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->itemFromIndex(index);
            const DocumentTreeNode entityNode = Internal::treeItemDocumentTreeNode(treeItem);
            entityNode.document()->destroyEntity(entityNode.id());
        }
    });

    this->connectTreeModelDataChanged(true);
}

WidgetModelTree::~WidgetModelTree()
{
    delete m_ui;
}

void WidgetModelTree::refreshItemText(const ApplicationItem& appItem)
{
    if (appItem.isDocument()) {
        const DocumentPtr doc = appItem.document();
        QTreeWidgetItem* treeItem = this->findTreeItem(doc);
        if (treeItem)
            this->findSupportBuilder(doc)->refreshTextTreeItem(doc, treeItem);
    }
    else if (appItem.isDocumentTreeNode()) {
        const DocumentTreeNode& node = appItem.documentTreeNode();
        QTreeWidgetItem* treeItemDocItem = this->findTreeItem(node);
        if (treeItemDocItem)
            this->findSupportBuilder(node)->refreshTextTreeItem(node, treeItemDocItem);
    }
}

void WidgetModelTree::registerGuiApplication(GuiApplication* guiApp)
{
    if (m_guiApp == guiApp)
        return;

    m_guiApp = guiApp;
    for (const BuilderPtr& builder : m_vecBuilder)
        builder->registerGuiApplication(guiApp);

    auto app = guiApp->application();
    QObject::connect(
                app.get(), &Application::documentAdded,
                this, &WidgetModelTree::onDocumentAdded);
    QObject::connect(
                app.get(), &Application::documentAboutToClose,
                this, &WidgetModelTree::onDocumentAboutToClose);
    QObject::connect(
                app.get(), &Application::documentNameChanged,
                this, &WidgetModelTree::onDocumentNameChanged);
    QObject::connect(
                app.get(), &Application::documentEntityAdded,
                this, &WidgetModelTree::onDocumentEntityAdded);
    QObject::connect(
                app.get(), &Application::documentEntityAboutToBeDestroyed,
                this, &WidgetModelTree::onDocumentEntityAboutToBeDestroyed);

    QObject::connect(m_guiApp, &GuiApplication::guiDocumentAdded, this, [=](GuiDocument* guiDoc) {
        QObject::connect(
                    guiDoc, &GuiDocument::nodesVisibilityChanged,
                    this, [=](const std::unordered_map<TreeNodeId, Qt::CheckState>& mapNodeId) {
            this->onNodesVisibilityChanged(guiDoc, mapNodeId);
        });
    });

    QObject::connect(
                m_guiApp->selectionModel(), &ApplicationItemSelectionModel::changed,
                this, &WidgetModelTree::onApplicationItemSelectionModelChanged);

    this->connectTreeWidgetDocumentSelectionChanged(true);
}

WidgetModelTree_UserActions WidgetModelTree::createUserActions(QObject* parent)
{
    WidgetModelTree_UserActions userActions;
    std::vector<WidgetModelTree_UserActions::FunctionSyncItems> vecFnSyncItems;
    for (const BuilderPtr& builder : m_vecBuilder) {
        const WidgetModelTree_UserActions subUserActions = builder->createUserActions(parent);
        for (QAction* action : subUserActions.items)
            userActions.items.push_back(action);

        if (subUserActions.fnSyncItems)
            vecFnSyncItems.push_back(std::move(subUserActions.fnSyncItems));
    }

    userActions.fnSyncItems = [=]{
        for (const WidgetModelTree_UserActions::FunctionSyncItems& fn : vecFnSyncItems)
            fn();
    };
    return userActions;
}

void WidgetModelTree::addPrototypeBuilder(BuilderPtr builder)
{
    Internal::arrayPrototypeBuilder().push_back(std::move(builder));
}

DocumentTreeNode WidgetModelTree::documentTreeNode(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemDocumentTreeNode(treeItem);
}

void WidgetModelTree::setDocumentTreeNode(QTreeWidgetItem* treeItem, const DocumentTreeNode& node)
{
    Internal::setTreeItemDocumentTreeNode(treeItem, node);
}

void WidgetModelTree::setDocument(QTreeWidgetItem* treeItem, const DocumentPtr& doc)
{
    Internal::setTreeItemDocument(treeItem, doc);
}

bool WidgetModelTree::holdsDocument(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemType(treeItem) == Internal::TreeItemType_Document;
}

bool WidgetModelTree::holdsDocumentTreeNode(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemType(treeItem) & Internal::TreeItemType_DocumentTreeNode;
}

void WidgetModelTree::onDocumentAdded(const DocumentPtr& doc)
{
    auto treeItem = this->findSupportBuilder(doc)->createTreeItem(doc);
    Internal::setTreeItemDocument(treeItem, doc);
    Q_ASSERT(Internal::treeItemDocument(treeItem) == doc);
    m_ui->treeWidget_Model->addTopLevelItem(treeItem);
}

void WidgetModelTree::onDocumentAboutToClose(const DocumentPtr& doc)
{
    delete this->findTreeItem(doc);
}

void WidgetModelTree::onDocumentNameChanged(const DocumentPtr& doc, const QString& /*name*/)
{
    QTreeWidgetItem* treeItem = this->findTreeItem(doc);
    if (treeItem)
        this->findSupportBuilder(doc)->refreshTextTreeItem(doc, treeItem);
}

QTreeWidgetItem* WidgetModelTree::loadDocumentEntity(const DocumentTreeNode& node)
{
    Expects(node.isEntity());
    auto treeItem = this->findSupportBuilder(node)->createTreeItem(node);
    Internal::setTreeItemDocumentTreeNode(treeItem, node);
    return treeItem;
}

QTreeWidgetItem* WidgetModelTree::findTreeItem(const DocumentPtr& doc) const
{
    for (int i = 0; i < m_ui->treeWidget_Model->topLevelItemCount(); ++i) {
        QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->topLevelItem(i);
        if (Internal::treeItemDocument(treeItem) == doc)
            return treeItem;
    }

    return nullptr;
}

QTreeWidgetItem* WidgetModelTree::findTreeItem(const DocumentTreeNode& node) const
{
    QTreeWidgetItem* treeItemDoc = this->findTreeItem(node.document());
    if (treeItemDoc) {
        for (QTreeWidgetItemIterator it(treeItemDoc); *it; ++it) {
            if (Internal::treeItemDocumentTreeNode(*it) == node)
                return *it;
        }
    }

    return nullptr;
}

WidgetModelTreeBuilder* WidgetModelTree::findSupportBuilder(const DocumentPtr& doc) const
{
    auto it = std::find_if(
                std::next(m_vecBuilder.cbegin()),
                m_vecBuilder.cend(),
                [=](const BuilderPtr& builder) { return builder->supportsDocument(doc); });
    return it != m_vecBuilder.cend() ? it->get() : m_vecBuilder.front().get();
}

WidgetModelTreeBuilder* WidgetModelTree::findSupportBuilder(const DocumentTreeNode& node) const
{
    Expects(node.isValid());
    auto it = std::find_if(
                std::next(m_vecBuilder.cbegin()),
                m_vecBuilder.cend(),
                [=](const BuilderPtr& builder) { return builder->supportsDocumentTreeNode(node); });
    return it != m_vecBuilder.cend() ? it->get() : m_vecBuilder.front().get();
}

void WidgetModelTree::onDocumentEntityAdded(const DocumentPtr& doc, TreeNodeId entityId)
{
    QTreeWidgetItem* treeDocEntity = this->loadDocumentEntity({ doc, entityId });
    QTreeWidgetItem* treeDoc = this->findTreeItem(doc);
    if (treeDoc) {
        treeDoc->addChild(treeDocEntity);
        treeDoc->setExpanded(true);
    }
}

void WidgetModelTree::onDocumentEntityAboutToBeDestroyed(const DocumentPtr& doc, TreeNodeId entityId)
{
    QTreeWidgetItem* treeItem = this->findTreeItem({ doc, entityId });
    delete treeItem;
}

//void WidgetModelTree::onDocumentItemPropertyChanged(
//        DocumentItem* docItem, Property* prop)
//{
//    QTreeWidgetItem* treeItem = this->findTreeItem(docItem);
//    if (treeItem && prop == &docItem->propertyLabel)
//        this->findSupportBuilder(docItem)->refreshTextTreeItem(docItem, treeItem);
//}

void WidgetModelTree::onTreeWidgetDocumentSelectionChanged(
        const QItemSelection& selected, const QItemSelection& deselected)
{
    const QModelIndexList listSelectedIndex = selected.indexes();
    const QModelIndexList listDeselectedIndex = deselected.indexes();
    std::vector<ApplicationItem> vecSelected;
    std::vector<ApplicationItem> vecDeselected;
    vecSelected.reserve(listSelectedIndex.size());
    vecDeselected.reserve(listDeselectedIndex.size());
    for (const QModelIndex& index : listSelectedIndex) {
        const QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->itemFromIndex(index);
        vecSelected.push_back(std::move(Internal::toApplicationItem(treeItem)));
    }

    for (const QModelIndex& index : listDeselectedIndex) {
        const QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->itemFromIndex(index);
        vecDeselected.push_back(std::move(Internal::toApplicationItem(treeItem)));
    }

    m_guiApp->selectionModel()->add(vecSelected);
    m_guiApp->selectionModel()->remove(vecDeselected);
}

void WidgetModelTree::onApplicationItemSelectionModelChanged(
        Span<ApplicationItem> selected, Span<ApplicationItem> deselected)
{
    this->connectTreeWidgetDocumentSelectionChanged(false);
    auto _ = gsl::finally([=] { this->connectTreeWidgetDocumentSelectionChanged(true); });

    auto fnSetSelected = [=](Span<ApplicationItem> spanAppItem, bool on) {
        for (const ApplicationItem& appItem : spanAppItem) {
            if (!appItem.isDocumentTreeNode())
                continue;

            QTreeWidgetItem* treeItem = this->findTreeItem(appItem.documentTreeNode());
            if (!treeItem)
                continue;

            treeItem->setSelected(on);
            if (on)
                m_ui->treeWidget_Model->scrollToItem(treeItem);
        }
    };

    fnSetSelected(selected, true);
    fnSetSelected(deselected, false);
}

void WidgetModelTree::connectTreeModelDataChanged(bool on)
{
    if (on) {
        m_connTreeModelDataChanged = QObject::connect(
                    m_ui->treeWidget_Model->model(), &QAbstractItemModel::dataChanged,
                    this, &WidgetModelTree::onTreeModelDataChanged, Qt::UniqueConnection);
    }
    else {
        QObject::disconnect(m_connTreeModelDataChanged);
    }
}

void WidgetModelTree::connectTreeWidgetDocumentSelectionChanged(bool on)
{
    if (on) {
        m_connTreeWidgetDocumentSelectionChanged = QObject::connect(
                    m_ui->treeWidget_Model->selectionModel(), &QItemSelectionModel::selectionChanged,
                    this, &WidgetModelTree::onTreeWidgetDocumentSelectionChanged,
                    Qt::UniqueConnection);
    }
    else {
        QObject::disconnect(m_connTreeWidgetDocumentSelectionChanged);
    }
}

void WidgetModelTree::onTreeModelDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    if (roles.contains(Qt::CheckStateRole) && topLeft == bottomRight) {
        const QModelIndex& indexItem = topLeft;
        const auto itemCheckState = Qt::CheckState(indexItem.data(Qt::CheckStateRole).toInt());
        if (itemCheckState == Qt::PartiallyChecked)
            return;

        const QVariant var = indexItem.data(Internal::TreeItemDocumentTreeNodeRole);
        auto treeNode = var.isValid() ? var.value<DocumentTreeNode>() : DocumentTreeNode::null();
        if (!treeNode.isValid())
            return;

        GuiDocument* guiDoc = m_guiApp->findGuiDocument(treeNode.document());
        if (!guiDoc)
            return;

        guiDoc->setNodeVisible(treeNode.id(), itemCheckState == Qt::Checked ? true : false);
        guiDoc->graphicsScene()->redraw();
    }
}

void WidgetModelTree::onNodesVisibilityChanged(
        const GuiDocument* guiDoc, const std::unordered_map<TreeNodeId, Qt::CheckState>& mapNodeId)
{
    QTreeWidgetItem* treeItemDoc = this->findTreeItem(guiDoc->document());
    if (!treeItemDoc)
        return;

    this->connectTreeModelDataChanged(false);
    auto _ = gsl::finally([=]{ this->connectTreeModelDataChanged(true); });

    auto localMapNodeId = mapNodeId;
    for (QTreeWidgetItemIterator it(treeItemDoc); *it; ++it) {
        QTreeWidgetItem* treeItem = *it;
        const DocumentTreeNode docTreeNode = Internal::treeItemDocumentTreeNode(treeItem);
        auto itNodeVisibleState = localMapNodeId.find(docTreeNode.id());
        if (itNodeVisibleState != localMapNodeId.cend()) {
            treeItem->setCheckState(0, itNodeVisibleState->second);
            localMapNodeId.erase(itNodeVisibleState);
        }

        if (localMapNodeId.empty())
            return;
    }
}

} // namespace Mayo
