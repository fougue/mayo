/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree.h"

#include "application.h"
#include "application_item_selection_model.h"
#include "caf_utils.h"
#include "string_utils.h"
#include "document.h"
#include "document_item.h"
#include "gui_application.h"
#include "mesh_item.h"
#include "theme.h"
#include "xde_document_item.h"

#include <fougtools/qttools/gui/item_view_buttons.h>

#include <QtCore/QMetaType>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <cassert>
#include <unordered_map>
#include <unordered_set>

Q_DECLARE_METATYPE(Mayo::DocumentItemNode)

namespace Mayo {
namespace Internal {

class TreeWidget : public QTreeWidget {
public:
    TreeWidget(QWidget* parent = nullptr)
        : QTreeWidget(parent)
    {}

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
    TreeItemDocumentItemRole,
    TreeItemDocumentItemNodeRole
};

enum TreeItemType {
    TreeItemType_Unknown = 0,
    TreeItemType_Document = 1,
    TreeItemType_DocumentItem = 2,
    TreeItemType_DocumentItemNode = 3
};

template<typename T> T* qVariantToPtr(const QVariant& var) {
    return static_cast<T*>(var.value<void*>());
}

template<typename T> QVariant ptrToQVariant(T* ptr) {
    return qVariantFromValue(reinterpret_cast<void*>(ptr));
}

static TreeItemType treeItemType(const QTreeWidgetItem* treeItem)
{
    const QVariant varType = treeItem->data(0, TreeItemTypeRole);
    return varType.isValid() ?
                static_cast<TreeItemType>(varType.toInt()) :
                TreeItemType_Unknown;
}

static Document* treeItemDocument(const QTreeWidgetItem* treeItem)
{
    const QVariant var = treeItem->data(0, TreeItemDocumentRole);
    return var.isValid() ? qVariantToPtr<Document>(var) : nullptr;
}

static DocumentItem* treeItemDocumentItem(const QTreeWidgetItem* treeItem)
{
    const QVariant var = treeItem->data(0, TreeItemDocumentItemRole);
    return var.isValid() ? qVariantToPtr<DocumentItem>(var) : nullptr;
}

static DocumentItemNode treeItemDocumentItemNode(const QTreeWidgetItem* treeItem)
{
    const QVariant var = treeItem->data(0, TreeItemDocumentItemNodeRole);
    return var.isValid() ? var.value<DocumentItemNode>() : DocumentItemNode::null();
}

static void setTreeItemDocument(QTreeWidgetItem* treeItem, Document* doc)
{
    treeItem->setData(0, TreeItemTypeRole, TreeItemType_Document);
    treeItem->setData(0, TreeItemDocumentRole, ptrToQVariant(doc));
}

static void setTreeItemDocumentItem(QTreeWidgetItem* treeItem, DocumentItem* docItem)
{
    treeItem->setData(0, TreeItemTypeRole, TreeItemType_DocumentItem);
    treeItem->setData(0, TreeItemDocumentItemRole, ptrToQVariant(docItem));
}

static void setTreeItemDocumentItemNode(
        QTreeWidgetItem* treeItem, const DocumentItemNode& node)
{
    treeItem->setData(0, TreeItemTypeRole, TreeItemType_DocumentItemNode);
    treeItem->setData(0, TreeItemDocumentItemNodeRole, QVariant::fromValue(node));
}

static QIcon documentItemIcon(const DocumentItem* docItem)
{
    if (sameType<XdeDocumentItem>(docItem))
        return mayoTheme()->icon(Theme::Icon::ItemXde);
    else if (sameType<MeshItem>(docItem))
        return mayoTheme()->icon(Theme::Icon::ItemMesh);

    return QIcon();
}

static QIcon xdeShapeIcon(const TDF_Label& label)
{
    if (XdeDocumentItem::isShapeAssembly(label))
        return mayoTheme()->icon(Theme::Icon::XdeAssembly);
    else if (XdeDocumentItem::isShapeReference(label))
        return QIcon(":/images/xde_reference_16.png");
    else if (XdeDocumentItem::isShapeSimple(label))
        return mayoTheme()->icon(Theme::Icon::XdeSimpleShape);

    return QIcon();
}

static QString itemLabelText(const PropertyQString& prop)
{
    const QString label = prop.value();
    return !label.isEmpty() ? label : WidgetModelTree::tr("<unnamed>");
}

static QTreeWidgetItem* guiCreateXdeTreeNode(
        QTreeWidgetItem* guiParentNode,
        XdeDocumentItem::AssemblyNodeId nodeId,
        XdeDocumentItem* xdeDocItem)
{
    auto guiNode = new QTreeWidgetItem(guiParentNode);
    const QString stdName = xdeDocItem->findLabelName(nodeId);
    guiNode->setText(0, stdName);
    setTreeItemDocumentItemNode(guiNode, DocumentItemNode(xdeDocItem, nodeId));
    const QIcon icon = xdeShapeIcon(xdeDocItem->assemblyTree().nodeData(nodeId));
    if (!icon.isNull())
        guiNode->setIcon(0, icon);

    return guiNode;
}

static ApplicationItem toApplicationItem(const QTreeWidgetItem* treeItem)
{
    const Internal::TreeItemType type = Internal::treeItemType(treeItem);
    if (type == Internal::TreeItemType_Document)
        return ApplicationItem(Internal::treeItemDocument(treeItem));
    else if (type == Internal::TreeItemType_DocumentItem)
        return ApplicationItem(Internal::treeItemDocumentItem(treeItem));
    else if (type == Internal::TreeItemType_DocumentItemNode)
        return ApplicationItem(Internal::treeItemDocumentItemNode(treeItem));

    return ApplicationItem();
}

} // namespace Internal

WidgetModelTree::WidgetModelTree(QWidget* widget)
    : QWidget(widget),
      m_ui(new Ui_WidgetModelTree)
{
    m_ui->setupUi(this);
    m_ui->treeWidget_Model->setUniformRowHeights(true);

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
                QVariant(Internal::TreeItemType_DocumentItem));
    modelTreeBtns->setButtonDisplayColumn(idBtnRemove, 0);
    modelTreeBtns->setButtonDisplayModes(
                idBtnRemove, qtgui::ItemViewButtons::DisplayOnDetection);
    modelTreeBtns->setButtonItemSide(
                idBtnRemove, qtgui::ItemViewButtons::ItemRightSide);
    const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    modelTreeBtns->setButtonIconSize(
                idBtnRemove, QSize(iconSize * 0.66, iconSize * 0.66));
    modelTreeBtns->installDefaultItemDelegate();
    QObject::connect(
                modelTreeBtns, &qtgui::ItemViewButtons::buttonClicked,
                [=](int btnId, const QModelIndex& index) {
        if (btnId == idBtnRemove) {
            QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->itemFromIndex(index);
            DocumentItem* docItem = Internal::treeItemDocumentItem(treeItem);
            docItem->document()->eraseRootItem(docItem);
        }
    });

    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &WidgetModelTree::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &WidgetModelTree::onDocumentErased);
    QObject::connect(
                app, &Application::documentPropertyChanged,
                this, &WidgetModelTree::onDocumentPropertyChanged);
    QObject::connect(
                app, &Application::documentItemAdded,
                this, &WidgetModelTree::onDocumentItemAdded);
    QObject::connect(
                app, &Application::documentItemErased,
                this, &WidgetModelTree::onDocumentItemErased);
    QObject::connect(
                app, &Application::documentItemPropertyChanged,
                this, &WidgetModelTree::onDocumentItemPropertyChanged);
    QObject::connect(
                m_ui->treeWidget_Model->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &WidgetModelTree::onTreeWidgetDocumentSelectionChanged);

    m_refItemTextTemplate = QString::fromUtf8("%instance");
}

WidgetModelTree::~WidgetModelTree()
{
    delete m_ui;
}

bool WidgetModelTree::isMergeXdeReferredShapeOn() const
{
    return m_isMergeXdeReferredShapeOn;
}

void WidgetModelTree::setMergeXdeReferredShape(bool on)
{
    m_isMergeXdeReferredShapeOn = on;
    // TODO : reload XDE documents
}

const QString& WidgetModelTree::referenceItemTextTemplate() const
{
    return m_refItemTextTemplate;
}

QString WidgetModelTree::referenceItemText(
        const TDF_Label& refLabel,
        const TDF_Label& referredLabel) const
{
    const QString refName = XdeDocumentItem::findLabelName(refLabel).trimmed();
    const QString referredName = XdeDocumentItem::findLabelName(referredLabel).trimmed();
    QString itemText = m_refItemTextTemplate;
    itemText.replace("%instance", refName)
            .replace("%referred", referredName);
    return itemText;
}

void WidgetModelTree::refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item)
{
    const DocumentItemNode docItemNode = Internal::treeItemDocumentItemNode(item);
    if (!sameType<XdeDocumentItem>(docItemNode.documentItem))
        return;

    const TDF_Label label = XdeDocumentItem::label(docItemNode);
    if (XdeDocumentItem::isShapeReference(label)) {
        const TDF_Label& refLabel = label;
        const TDF_Label referredLabel = XdeDocumentItem::shapeReferred(refLabel);
        const QString itemText = this->referenceItemText(refLabel, referredLabel);
        item->setText(0, itemText);
    }
    else {
        item->setText(0, XdeDocumentItem::findLabelName(label));
    }
}

void WidgetModelTree::setReferenceItemTextTemplate(const QString& textTemplate)
{
    m_refItemTextTemplate = textTemplate;
    for (QTreeWidgetItemIterator it(m_ui->treeWidget_Model); *it; ++it) {
        if (Internal::treeItemType(*it) == Internal::TreeItemType_DocumentItemNode)
            this->refreshXdeAssemblyNodeItemText(*it);
    }
}

void WidgetModelTree::refreshItemText(const ApplicationItem& appItem)
{
    if (appItem.isDocument()) {
        const Document* doc = appItem.document();
        QTreeWidgetItem* treeItem = this->findTreeItemDocument(doc);
        if (treeItem)
            treeItem->setText(0, Internal::itemLabelText(doc->propertyLabel));
    }
    else if (appItem.isDocumentItem()) {
        const DocumentItem* docItem = appItem.documentItem();
        QTreeWidgetItem* treeItem = this->findTreeItemDocumentItem(docItem);
        if (treeItem)
            treeItem->setText(0, Internal::itemLabelText(docItem->propertyLabel));
    }
    else if (appItem.isDocumentItemNode() && sameType<XdeDocumentItem>(appItem.documentItem())) {
        const TDF_Label labelNode = XdeDocumentItem::label(appItem.documentItemNode());
        TDF_LabelSequence seqLabelRefresh;
        if (XdeDocumentItem::isShapeReference(labelNode)
                && m_refItemTextTemplate.contains("%referred"))
        {
            XCAFDoc_ShapeTool::GetUsers(
                        XdeDocumentItem::shapeReferred(labelNode),
                        seqLabelRefresh,
                        false /* don't get sub children */);
        }
        else {
            seqLabelRefresh.Append(labelNode);
        }

        for (const TDF_Label& labelRefresh : seqLabelRefresh) {
            QTreeWidgetItem* treeItem =
                    this->findTreeItemXdeLabel(appItem.documentItem(), labelRefresh);
            if (treeItem)
                this->refreshXdeAssemblyNodeItemText(treeItem);
        }
    }
}

void WidgetModelTree::onDocumentAdded(Document* doc)
{
    auto treeItem = new QTreeWidgetItem;
    treeItem->setText(0, Internal::itemLabelText(doc->propertyLabel));
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::File));
    treeItem->setToolTip(0, doc->filePath());
    Internal::setTreeItemDocument(treeItem, doc);
    assert(Internal::treeItemDocument(treeItem) == doc);
    m_ui->treeWidget_Model->addTopLevelItem(treeItem);
}

void WidgetModelTree::onDocumentErased(const Document* doc)
{
    delete this->findTreeItemDocument(doc);
}

void WidgetModelTree::onDocumentPropertyChanged(Document* doc, Property* prop)
{
    QTreeWidgetItem* treeItem = this->findTreeItemDocument(doc);
    if (treeItem && prop == &doc->propertyLabel)
        treeItem->setText(0, Internal::itemLabelText(doc->propertyLabel));
}

QTreeWidgetItem* WidgetModelTree::loadDocumentItem(DocumentItem* docItem)
{
    auto treeItem = new QTreeWidgetItem;
    treeItem->setText(0, Internal::itemLabelText(docItem->propertyLabel));
    const QIcon docItemIcon = Internal::documentItemIcon(docItem);
    if (!docItemIcon.isNull())
        treeItem->setIcon(0, docItemIcon);
    Internal::setTreeItemDocumentItem(treeItem, docItem);
    return treeItem;
}

void WidgetModelTree::guiBuildXdeTree(
        QTreeWidgetItem* treeDocItem, XdeDocumentItem* xdeDocItem)
{
    std::unordered_map<TreeNodeId, QTreeWidgetItem*> mapNodeIdToTreeItem;
    std::unordered_set<TreeNodeId> setRefNodeId;
    const Tree<TDF_Label>& asmTree = xdeDocItem->assemblyTree();
    deepForeachTreeNode(asmTree, [&](TreeNodeId nodeId) {
        const TreeNodeId nodeParentId = asmTree.nodeParent(nodeId);
        const TDF_Label& nodeLabel = asmTree.nodeData(nodeId);
        auto itParentFound = mapNodeIdToTreeItem.find(nodeParentId);
        QTreeWidgetItem* guiParentNode =
                itParentFound != mapNodeIdToTreeItem.end() ?
                    itParentFound->second : treeDocItem;
        if (m_isMergeXdeReferredShapeOn) {
            if (XdeDocumentItem::isShapeReference(nodeLabel)) {
                mapNodeIdToTreeItem.insert({ nodeId, guiParentNode });
                setRefNodeId.insert(nodeId);
            }
            else {
                auto guiNode = new QTreeWidgetItem(guiParentNode);
                QString guiNodeText = XdeDocumentItem::findLabelName(nodeLabel);
                XdeDocumentItem::AssemblyNodeId guiNodeId = nodeId;
                if (setRefNodeId.find(nodeParentId) != setRefNodeId.cend()) {
                    const TDF_Label& refLabel = asmTree.nodeData(nodeParentId);
                    guiNodeText = this->referenceItemText(refLabel, nodeLabel);
                    guiNodeId = nodeParentId;
                }
                guiNode->setText(0, guiNodeText);
                Internal::setTreeItemDocumentItemNode(
                            guiNode, DocumentItemNode(xdeDocItem, guiNodeId));
                const QIcon icon = Internal::xdeShapeIcon(nodeLabel);
                if (!icon.isNull())
                    guiNode->setIcon(0, icon);
                mapNodeIdToTreeItem.insert({ nodeId, guiNode });
            }
        }
        else {
            auto guiNode = Internal::guiCreateXdeTreeNode(guiParentNode, nodeId, xdeDocItem);
            mapNodeIdToTreeItem.insert({ nodeId, guiNode });
        }
    });
}

QTreeWidgetItem* WidgetModelTree::findTreeItemDocument(const Document* doc) const
{
    for (int i = 0; i < m_ui->treeWidget_Model->topLevelItemCount(); ++i) {
        QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->topLevelItem(i);
        if (Internal::treeItemDocument(treeItem) == doc)
            return treeItem;
    }

    return nullptr;
}

QTreeWidgetItem* WidgetModelTree::findTreeItemDocumentItem(const DocumentItem* docItem) const
{
    QTreeWidgetItem* treeItemDoc = this->findTreeItemDocument(docItem->document());
    if (treeItemDoc) {
        for (QTreeWidgetItemIterator it(treeItemDoc); *it; ++it) {
            if (Internal::treeItemDocumentItem(*it) == docItem)
                return *it;
        }
    }

    return nullptr;
}

QTreeWidgetItem* WidgetModelTree::findTreeItemXdeLabel(
        const DocumentItem* docItem, const TDF_Label& label) const
{
    QTreeWidgetItem* treeItem = this->findTreeItemDocumentItem(docItem);
    if (!treeItem)
        return nullptr;

    for (QTreeWidgetItemIterator it(treeItem); *it; ++it) {
        const DocumentItemNode docItemNode = Internal::treeItemDocumentItemNode(*it);
        if (XdeDocumentItem::label(docItemNode) == label)
            return *it;
    }

    return nullptr;
}

void WidgetModelTree::onDocumentItemAdded(DocumentItem* docItem)
{
    QTreeWidgetItem* treeDocItem = this->loadDocumentItem(docItem);
    if (sameType<XdeDocumentItem>(docItem)) {
        auto xdeDocItem = static_cast<XdeDocumentItem*>(docItem);
        this->guiBuildXdeTree(treeDocItem, xdeDocItem);
    }

    QTreeWidgetItem* treeItemDoc = this->findTreeItemDocument(docItem->document());
    if (treeItemDoc) {
        treeItemDoc->addChild(treeDocItem);
        treeItemDoc->setExpanded(true);
    }
}

void WidgetModelTree::onDocumentItemErased(const DocumentItem* docItem)
{
    QTreeWidgetItem* treeItem = this->findTreeItemDocumentItem(docItem);
    delete treeItem;
}

void WidgetModelTree::onDocumentItemPropertyChanged(
        DocumentItem* docItem, Property* prop)
{
    QTreeWidgetItem* treeItem = this->findTreeItemDocumentItem(docItem);
    if (treeItem &&prop == &docItem->propertyLabel)
        treeItem->setText(0, Internal::itemLabelText(docItem->propertyLabel));
}

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
    //emit selectionChanged();
}

} // namespace Mayo
