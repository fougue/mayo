/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree.h"

#include "../base/application.h"
#include "../base/application_item_selection_model.h"
#include "../base/string_utils.h"
#include "../base/document.h"
#include "../base/document_item.h"
#include "../gui/gui_application.h"
#include "theme.h"
#include "widget_model_tree_builder.h"

#include <fougtools/qttools/gui/item_view_buttons.h>

#include <QtCore/QMetaType>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <cassert>
#include <memory>

Q_DECLARE_METATYPE(Mayo::DocumentItemNode)

namespace Mayo {
namespace Internal {

using PtrBuilder = std::unique_ptr<WidgetModelTreeBuilder>;
static std::vector<PtrBuilder>& arrayPrototypeBuilder()
{
    static std::vector<PtrBuilder> vecPtrBuilder;
    if (vecPtrBuilder.empty())
        vecPtrBuilder.emplace_back(new WidgetModelTreeBuilder); // Fallback

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
                static_cast<Internal::TreeItemType>(varType.toInt()) :
                Internal::TreeItemType_Unknown;
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
    treeItem->setData(0, TreeItemTypeRole, Internal::TreeItemType_Document);
    treeItem->setData(0, TreeItemDocumentRole, ptrToQVariant(doc));
}

static void setTreeItemDocumentItem(QTreeWidgetItem* treeItem, DocumentItem* docItem)
{
    treeItem->setData(0, TreeItemTypeRole, Internal::TreeItemType_DocumentItem);
    treeItem->setData(0, TreeItemDocumentItemRole, ptrToQVariant(docItem));
}

static void setTreeItemDocumentItemNode(
        QTreeWidgetItem* treeItem, const DocumentItemNode& node)
{
    treeItem->setData(0, TreeItemTypeRole, Internal::TreeItemType_DocumentItemNode);
    treeItem->setData(0, TreeItemDocumentItemNodeRole, QVariant::fromValue(node));
}

static ApplicationItem toApplicationItem(const QTreeWidgetItem* treeItem)
{
    const TreeItemType type = Internal::treeItemType(treeItem);
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
    for (const Internal::PtrBuilder& ptrBuilder : Internal::arrayPrototypeBuilder()) {
        m_vecBuilder.push_back(ptrBuilder->clone());
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
}

WidgetModelTree::~WidgetModelTree()
{
    delete m_ui;
    for (WidgetModelTreeBuilder* builder : m_vecBuilder)
        delete builder;
}

void WidgetModelTree::refreshItemText(const ApplicationItem& appItem)
{
    if (appItem.isDocument()) {
        const Document* doc = appItem.document();
        QTreeWidgetItem* treeItem = this->findTreeItem(doc);
        if (treeItem)
            this->findSupportBuilder(doc)->refreshTextTreeItem(doc, treeItem);
    }
    else if (appItem.isDocumentItem()) {
        const DocumentItem* docItem = appItem.documentItem();
        QTreeWidgetItem* treeItem = this->findTreeItem(docItem);
        if (treeItem)
            this->findSupportBuilder(docItem)->refreshTextTreeItem(docItem, treeItem);
    }
    else if (appItem.isDocumentItemNode()) {
        const DocumentItem* docItem = appItem.documentItem();
        const DocumentItemNode& node = appItem.documentItemNode();
        QTreeWidgetItem* treeItemDocItem = this->findTreeItem(docItem);
        if (treeItemDocItem)
            this->findSupportBuilder(docItem)->refreshTextTreeItem(node, treeItemDocItem);
    }
}

void WidgetModelTree::loadConfiguration(const QSettings* settings, const QString& keyGroup)
{
    const QString keyModel = keyGroup + "/" + this->objectName();
    for (WidgetModelTreeBuilder* builder : m_vecBuilder)
        builder->loadConfiguration(settings, keyModel);
}

void WidgetModelTree::saveConfiguration(QSettings* settings, const QString& keyGroup)
{
    const QString keyModel = keyGroup + "/" + this->objectName();
    for (WidgetModelTreeBuilder* builder : m_vecBuilder)
        builder->saveConfiguration(settings, keyModel);
}

std::vector<QAction*> WidgetModelTree::createConfigurationActions(QObject* parent)
{
    std::vector<QAction*> vecAction;
    for (WidgetModelTreeBuilder* builder : m_vecBuilder) {
        for (QAction* action : builder->createConfigurationActions(parent))
            vecAction.push_back(action);
    }

    return vecAction;
}

void WidgetModelTree::addPrototypeBuilder(WidgetModelTreeBuilder* builder)
{
    Internal::arrayPrototypeBuilder().emplace_back(builder);
}

DocumentItemNode WidgetModelTree::documentItemNode(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemDocumentItemNode(treeItem);
}

void WidgetModelTree::setDocumentItemNode(
        QTreeWidgetItem* treeItem, const DocumentItemNode& node)
{
    Internal::setTreeItemDocumentItemNode(treeItem, node);
}

bool WidgetModelTree::holdsDocument(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemType(treeItem) == Internal::TreeItemType_Document;
}

bool WidgetModelTree::holdsDocumentItem(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemType(treeItem) == Internal::TreeItemType_DocumentItem;
}

bool WidgetModelTree::holdsDocumentItemNode(const QTreeWidgetItem* treeItem)
{
    return Internal::treeItemType(treeItem) == Internal::TreeItemType_DocumentItemNode;
}

void WidgetModelTree::onDocumentAdded(Document* doc)
{
    auto treeItem = new QTreeWidgetItem;
    this->findSupportBuilder(doc)->fillTreeItem(treeItem, doc);
    Internal::setTreeItemDocument(treeItem, doc);
    Q_ASSERT(Internal::treeItemDocument(treeItem) == doc);
    m_ui->treeWidget_Model->addTopLevelItem(treeItem);
}

void WidgetModelTree::onDocumentErased(const Document* doc)
{
    delete this->findTreeItem(doc);
}

void WidgetModelTree::onDocumentPropertyChanged(Document* doc, Property* prop)
{
    QTreeWidgetItem* treeItem = this->findTreeItem(doc);
    if (treeItem && prop == &doc->propertyLabel)
        this->findSupportBuilder(doc)->refreshTextTreeItem(doc, treeItem);
}

QTreeWidgetItem* WidgetModelTree::loadDocumentItem(DocumentItem* docItem)
{
    auto treeItem = new QTreeWidgetItem;
    Internal::setTreeItemDocumentItem(treeItem, docItem);
    this->findSupportBuilder(docItem)->fillTreeItem(treeItem, docItem);
    return treeItem;
}

QTreeWidgetItem* WidgetModelTree::findTreeItem(const Document* doc) const
{
    for (int i = 0; i < m_ui->treeWidget_Model->topLevelItemCount(); ++i) {
        QTreeWidgetItem* treeItem = m_ui->treeWidget_Model->topLevelItem(i);
        if (Internal::treeItemDocument(treeItem) == doc)
            return treeItem;
    }

    return nullptr;
}

QTreeWidgetItem* WidgetModelTree::findTreeItem(const DocumentItem* docItem) const
{
    QTreeWidgetItem* treeItemDoc = this->findTreeItem(docItem->document());
    if (treeItemDoc) {
        for (QTreeWidgetItemIterator it(treeItemDoc); *it; ++it) {
            if (Internal::treeItemDocumentItem(*it) == docItem)
                return *it;
        }
    }

    return nullptr;
}

WidgetModelTreeBuilder* WidgetModelTree::findSupportBuilder(const Document* doc) const
{
    auto it = std::find_if(
                std::next(m_vecBuilder.cbegin()),
                m_vecBuilder.cend(),
                [=](WidgetModelTreeBuilder* builder) { return builder->supports(doc); });
    return it != m_vecBuilder.cend() ? *it : m_vecBuilder.front();
}

WidgetModelTreeBuilder* WidgetModelTree::findSupportBuilder(const DocumentItem* docItem) const
{
    auto it = std::find_if(
                std::next(m_vecBuilder.cbegin()),
                m_vecBuilder.cend(),
                [=](WidgetModelTreeBuilder* builder) { return builder->supports(docItem); });
    return it != m_vecBuilder.cend() ? *it : m_vecBuilder.front();
}

void WidgetModelTree::onDocumentItemAdded(DocumentItem* docItem)
{
    QTreeWidgetItem* treeDocItem = this->loadDocumentItem(docItem);
    QTreeWidgetItem* treeDoc = this->findTreeItem(docItem->document());
    if (treeDoc) {
        treeDoc->addChild(treeDocItem);
        treeDoc->setExpanded(true);
    }
}

void WidgetModelTree::onDocumentItemErased(const DocumentItem* docItem)
{
    QTreeWidgetItem* treeItem = this->findTreeItem(docItem);
    delete treeItem;
}

void WidgetModelTree::onDocumentItemPropertyChanged(
        DocumentItem* docItem, Property* prop)
{
    QTreeWidgetItem* treeItem = this->findTreeItem(docItem);
    if (treeItem && prop == &docItem->propertyLabel)
        this->findSupportBuilder(docItem)->refreshTextTreeItem(docItem, treeItem);
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
