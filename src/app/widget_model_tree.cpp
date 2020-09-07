/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
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
#include "theme.h"
#include "widget_model_tree_builder.h"

#include <fougtools/qttools/gui/item_view_buttons.h>

#include <QtCore/QMetaType>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <cassert>
#include <memory>

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

//template<typename T> T* qVariantToPtr(const QVariant& var) {
//    return static_cast<T*>(var.value<void*>());
//}

//template<typename T> QVariant ptrToQVariant(T* ptr) {
//    return qVariantFromValue(reinterpret_cast<void*>(ptr));
//}

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
    auto modelTreeBtns = new qtgui::ItemViewButtons(m_ui->treeWidget_Model, this);
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
    modelTreeBtns->setButtonDisplayModes(idBtnRemove, qtgui::ItemViewButtons::DisplayOnDetection);
    modelTreeBtns->setButtonItemSide(idBtnRemove, qtgui::ItemViewButtons::ItemRightSide);
    const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    modelTreeBtns->setButtonIconSize(idBtnRemove, QSize(iconSize * 0.66, iconSize * 0.66));
    modelTreeBtns->installDefaultItemDelegate();
    QObject::connect(
                modelTreeBtns, &qtgui::ItemViewButtons::buttonClicked,
                this, [=](int btnId, const QModelIndex& index) {
        if (btnId == idBtnRemove && index.isValid()) {
            const QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->itemFromIndex(index);
            const DocumentTreeNode entityNode = Internal::treeItemDocumentTreeNode(treeItem);
            entityNode.document()->destroyEntity(entityNode.id());
        }
    });

    auto app = Application::instance();
    QObject::connect(
                app.get(), &Application::documentAdded,
                this, &WidgetModelTree::onDocumentAdded);
    QObject::connect(
                app.get(), &Application::documentAboutToClose,
                this, &WidgetModelTree::onDocumentAboutToClose);
//    QObject::connect(
//                app, &Application::documentPropertyChanged,
//                this, &WidgetModelTree::onDocumentPropertyChanged);
    QObject::connect(
                app.get(), &Application::documentNameChanged,
                this, &WidgetModelTree::onDocumentNameChanged);
    QObject::connect(
                app.get(), &Application::documentEntityAdded,
                this, &WidgetModelTree::onDocumentEntityAdded);
    QObject::connect(
                app.get(), &Application::documentEntityAboutToBeDestroyed,
                this, &WidgetModelTree::onDocumentEntityAboutToBeDestroyed);
//    QObject::connect(
//                app, &Application::documentItemPropertyChanged,
//                this, &WidgetModelTree::onDocumentItemPropertyChanged);
    QObject::connect(
                m_ui->treeWidget_Model->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &WidgetModelTree::onTreeWidgetDocumentSelectionChanged);
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

void WidgetModelTree::loadConfiguration(const Settings* settings, const QString& keyGroup)
{
    const QString keyModel = keyGroup + "/" + this->objectName();
    for (const BuilderPtr& builder : m_vecBuilder)
        builder->loadConfiguration(settings, keyModel);
}

void WidgetModelTree::saveConfiguration(Settings* settings, const QString& keyGroup)
{
    const QString keyModel = keyGroup + "/" + this->objectName();
    for (const BuilderPtr& builder : m_vecBuilder)
        builder->saveConfiguration(settings, keyModel);
}

std::vector<QAction*> WidgetModelTree::createConfigurationActions(QObject* parent)
{
    std::vector<QAction*> vecAction;
    for (const BuilderPtr& builder : m_vecBuilder) {
        for (QAction* action : builder->createConfigurationActions(parent))
            vecAction.push_back(action);
    }

    return vecAction;
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

//void WidgetModelTree::onDocumentPropertyChanged(Document* doc, Property* prop)
//{
//    QTreeWidgetItem* treeItem = this->findTreeItem(doc);
//    if (treeItem && prop == &doc->propertyLabel)
//        this->findSupportBuilder(doc)->refreshTextTreeItem(doc, treeItem);
//}

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

    GuiApplication::instance()->selectionModel()->add(vecSelected);
    GuiApplication::instance()->selectionModel()->remove(vecDeselected);
}

} // namespace Mayo
