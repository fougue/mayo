/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "widget_application_tree.h"

#include "application.h"
#include "caf_utils.h"
#include "string_utils.h"
#include "document.h"
#include "document_item.h"
#include "mesh_item.h"
#include "ui_widget_application_tree.h"
#include "xde_document_item.h"
#include "xde_shape_explorer.h"

#include <QtCore/QMetaType>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <cassert>
#include <unordered_map>

//#define MAYO_WIDGET_APPLICATION_TREE_SHOW_REFERENCE_NODES

Q_DECLARE_METATYPE(Mayo::XdeDocumentItem::Label)

namespace Mayo {

namespace Internal {

enum TreeItemRole {
    TreeItemTypeRole = Qt::UserRole + 1,
    TreeItemDocumentRole,
    TreeItemDocumentItemRole,
    TreeItemXdeDocumentItemLabelRole
};

enum TreeItemType {
    TreeItemType_Unknown = 0,
    TreeItemType_Document = 1,
    TreeItemType_DocumentItem = 2,
    TreeItemType_XdeDocumentItemLabel = 3
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

static XdeDocumentItem::Label treeItemXdeDocumentItemLabel(
        const QTreeWidgetItem* treeItem)
{
    const QVariant var = treeItem->data(0, TreeItemXdeDocumentItemLabelRole);
    return var.isValid() ?
                var.value<XdeDocumentItem::Label>() :
                XdeDocumentItem::Label::null();
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

static void setTreeItemXdeDocumentItemLabel(
        QTreeWidgetItem* treeItem, const XdeDocumentItem::Label& lbl)
{
    treeItem->setData(0, TreeItemTypeRole, TreeItemType_XdeDocumentItemLabel);
    treeItem->setData(0, TreeItemXdeDocumentItemLabelRole, QVariant::fromValue(lbl));
}

static QIcon documentItemIcon(const DocumentItem* docItem)
{
    if (sameType<XdeDocumentItem>(docItem))
        return QIcon(":/images/xde_document_16.png");
    else if (sameType<MeshItem>(docItem))
        return QIcon(":/images/mesh_16.png");
    return QIcon();
}

static QIcon xdeShapeIcon(const XdeDocumentItem* docItem, const TDF_Label& label)
{
    if (docItem->isShapeAssembly(label))
        return QIcon(":/images/xde_assembly_16.png");
    else if (docItem->isShapeReference(label))
        return QIcon(":/images/xde_reference_16.png");
    else if (docItem->isShapeSimple(label))
        return QIcon(":/images/xde_simple_shape_16.png");
    return QIcon();
}

static QString documentItemLabel(const DocumentItem* docItem)
{
    const QString docItemLabel = docItem->propertyLabel.value();
    return !docItemLabel.isEmpty() ?
                docItemLabel :
                WidgetApplicationTree::tr("<unnamed>");
}

static void addValidationProperties(
        const XdeDocumentItem::ValidationProperties& validationProps,
        std::vector<HandleProperty>* ptrVecHndProp,
        const QString& nameFormat = QStringLiteral("%1"),
        HandleProperty::Storage hndStorage = HandleProperty::Owner,
        PropertyOwner* propOwner = nullptr)
{
    if (validationProps.hasCentroid) {
        auto propCentroid = new PropertyOccPnt(
                    propOwner, nameFormat.arg(WidgetApplicationTree::tr("Centroid")));
        propCentroid->setValue(validationProps.centroid);
        ptrVecHndProp->emplace_back(propCentroid, hndStorage);
    }
    if (validationProps.hasArea) {
        auto propArea = new PropertyArea(
                    propOwner, nameFormat.arg(WidgetApplicationTree::tr("Area")));
        propArea->setQuantity(validationProps.area);
        ptrVecHndProp->emplace_back(propArea, hndStorage);
    }
    if (validationProps.hasVolume) {
        auto propVolume = new PropertyVolume(
                    propOwner, nameFormat.arg(WidgetApplicationTree::tr("Volume")));
        propVolume->setQuantity(validationProps.volume);
        ptrVecHndProp->emplace_back(propVolume, hndStorage);
    }
}

} // namespace Internal

WidgetApplicationTree::WidgetApplicationTree(QWidget *widget)
    : QWidget(widget),
      m_ui(new Ui_WidgetApplicationTree)
{
    m_ui->setupUi(this);

    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &WidgetApplicationTree::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &WidgetApplicationTree::onDocumentErased);
    QObject::connect(
                app, &Application::documentItemAdded,
                this, &WidgetApplicationTree::onDocumentItemAdded);
    QObject::connect(
                app, &Application::documentItemPropertyChanged,
                this, &WidgetApplicationTree::onDocumentItemPropertyChanged);
    QObject::connect(
                m_ui->treeWidget_App->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &WidgetApplicationTree::onTreeWidgetDocumentSelectionChanged);
}

WidgetApplicationTree::~WidgetApplicationTree()
{
    delete m_ui;
}

std::vector<WidgetApplicationTree::Item> WidgetApplicationTree::selectedItems() const
{
    const QList<QTreeWidgetItem*> listTreeItem =
            m_ui->treeWidget_App->selectedItems();
    std::vector<Item> vecItem;
    vecItem.reserve(listTreeItem.size());
    for (const QTreeWidgetItem* treeItem : listTreeItem) {
        const Internal::TreeItemType type = Internal::treeItemType(treeItem);
        if (type == Internal::TreeItemType_Document)
            vecItem.emplace_back(Internal::treeItemDocument(treeItem));
        else if (type == Internal::TreeItemType_DocumentItem)
            vecItem.emplace_back(Internal::treeItemDocumentItem(treeItem));
        else if (type == Internal::TreeItemType_XdeDocumentItemLabel)
            vecItem.emplace_back(Internal::treeItemXdeDocumentItemLabel(treeItem));
    }
    return vecItem;
}

bool WidgetApplicationTree::hasSelectedDocumentItems() const
{
    const QList<QTreeWidgetItem*> listTreeItem =
            m_ui->treeWidget_App->selectedItems();
    for (const QTreeWidgetItem* treeItem : listTreeItem) {
        if (Internal::treeItemType(treeItem) == Internal::TreeItemType_DocumentItem)
            return true;
    }
    return false;
}

std::vector<DocumentItem*> WidgetApplicationTree::selectedDocumentItems() const
{
    const QList<QTreeWidgetItem*> listTreeItem =
            m_ui->treeWidget_App->selectedItems();
    std::vector<DocumentItem*> vecDocItem;
    vecDocItem.reserve(listTreeItem.size());
    for (const QTreeWidgetItem* treeItem : listTreeItem) {
        if (Internal::treeItemType(treeItem) == Internal::TreeItemType_DocumentItem)
            vecDocItem.push_back(Internal::treeItemDocumentItem(treeItem));
    }
    return vecDocItem;
}

bool WidgetApplicationTree::isMergeXdeReferredShapeOn() const
{
    return m_isMergeXdeReferredShapeOn;
}

void WidgetApplicationTree::setMergeXdeReferredShape(bool on)
{
    m_isMergeXdeReferredShapeOn = on;
    // TODO : reload XDE documents
}

void WidgetApplicationTree::onDocumentAdded(Document *doc)
{
    auto treeItem = new QTreeWidgetItem;
    const QString docLabel =
            !doc->label().isEmpty() ? doc->label() : tr("<unnamed>");
    treeItem->setText(0, docLabel);
    treeItem->setIcon(0, QPixmap(":/images/file_16.png"));
    Internal::setTreeItemDocument(treeItem, doc);
    assert(Internal::treeItemDocument(treeItem) == doc);
    m_ui->treeWidget_App->addTopLevelItem(treeItem);
}

void WidgetApplicationTree::onDocumentErased(const Document *doc)
{
    delete this->findTreeItemDocument(doc);
}

QTreeWidgetItem* WidgetApplicationTree::loadDocumentItem(DocumentItem* docItem)
{
    auto treeItem = new QTreeWidgetItem;
    const QString docItemLabel = Internal::documentItemLabel(docItem);
    treeItem->setText(0, docItemLabel);
    const QIcon docItemIcon = Internal::documentItemIcon(docItem);
    if (!docItemIcon.isNull())
        treeItem->setIcon(0, docItemIcon);
    Internal::setTreeItemDocumentItem(treeItem, docItem);
    return treeItem;
}

void WidgetApplicationTree::loadXdeShapeStructure(
        QTreeWidgetItem *treeDocItem, XdeDocumentItem *xdeDocItem)
{
    std::unordered_map<unsigned, QTreeWidgetItem*> idToTreeItem;
    std::unordered_map<unsigned, TDF_Label> idToRefLabel;
    const TDF_LabelSequence seqShapeLabel =
            xdeDocItem->topLevelFreeShapeLabels();
    for (const TDF_Label& rootLabel : seqShapeLabel) {
        XdeShapeExplorer expl(xdeDocItem->shapeTool(), rootLabel);
        while (!expl.atEnd()) {
            const TDF_Label& currentLabel = expl.current();
            const auto itParentTreeItem =
                    idToTreeItem.find(expl.currentParentIterationId());
            QTreeWidgetItem* parentTreeItem =
                    itParentTreeItem != idToTreeItem.cend() ?
                        itParentTreeItem->second :
                        treeDocItem;
            if (m_isMergeXdeReferredShapeOn) {
                // Hide references (show "referred" shapes instead)
                if (xdeDocItem->isShapeReference(currentLabel)) {
                    idToRefLabel.insert({ expl.currentIterationId(), currentLabel });
                    idToTreeItem.insert({ expl.currentIterationId(), parentTreeItem });
                }
                else {
                    auto treeItem = new QTreeWidgetItem(parentTreeItem);
                    const QString stdName = xdeDocItem->findLabelName(currentLabel);

                    const auto itRef =
                            idToRefLabel.find(expl.currentParentIterationId());
                    if (itRef != idToRefLabel.cend()) {
                        const TDF_Label& refLabel = itRef->second;
                        const QString refStdName =
                                xdeDocItem->findLabelName(refLabel).trimmed();
                        const QString text =
                                !refStdName.isEmpty() && refStdName != stdName ?
                                    // \xe2\x86\x92 : UTF8 rightwards arrow
                                    QString::fromUtf8("%1 [\xe2\x86\x92%2]").arg(refStdName, stdName) :
                                    stdName;
                        treeItem->setText(0, text);
                        Internal::setTreeItemXdeDocumentItemLabel(
                                    treeItem,
                                    XdeDocumentItem::Label(xdeDocItem, refLabel));
                    }
                    else {
                        treeItem->setText(0, stdName);
                        Internal::setTreeItemXdeDocumentItemLabel(
                                    treeItem,
                                    XdeDocumentItem::Label(xdeDocItem, currentLabel));
                    }

                    const QIcon icon = Internal::xdeShapeIcon(xdeDocItem, currentLabel);
                    if (!icon.isNull())
                        treeItem->setIcon(0, icon);
                    idToTreeItem.insert({ expl.currentIterationId(), treeItem });
                }
            }
            else {
                auto treeItem = new QTreeWidgetItem(parentTreeItem);
                const QString stdName = xdeDocItem->findLabelName(currentLabel);
                treeItem->setText(0, stdName);
                Internal::setTreeItemXdeDocumentItemLabel(
                            treeItem,
                            XdeDocumentItem::Label(xdeDocItem, currentLabel));
                const QIcon icon = Internal::xdeShapeIcon(xdeDocItem, currentLabel);
                if (!icon.isNull())
                    treeItem->setIcon(0, icon);
                idToTreeItem.insert({ expl.currentIterationId(), treeItem });
            }
            expl.goNext();
        }
    }
}

QTreeWidgetItem *WidgetApplicationTree::findTreeItemDocument(const Document *doc) const
{
    for (int i = 0; i < m_ui->treeWidget_App->topLevelItemCount(); ++i) {
        QTreeWidgetItem* treeItem = m_ui->treeWidget_App->topLevelItem(i);
        if (Internal::treeItemDocument(treeItem) == doc)
            return treeItem;
    }
    return nullptr;
}

QTreeWidgetItem *WidgetApplicationTree::findTreeItemDocumentItem(const DocumentItem *docItem) const
{
    QTreeWidgetItem* treeItemDoc = this->findTreeItemDocument(docItem->document());
    if (treeItemDoc != nullptr) {
        for (QTreeWidgetItemIterator it(treeItemDoc); *it; ++it) {
            if (Internal::treeItemDocumentItem(*it) == docItem)
                return *it;
        }
    }
    return nullptr;
}

void WidgetApplicationTree::onDocumentItemAdded(DocumentItem *docItem)
{
    QTreeWidgetItem* treeDocItem = this->loadDocumentItem(docItem);
    if (sameType<XdeDocumentItem>(docItem)) {
        auto xdeDocItem = static_cast<XdeDocumentItem*>(docItem);
        this->loadXdeShapeStructure(treeDocItem, xdeDocItem);
    }
    QTreeWidgetItem* treeItemDoc = this->findTreeItemDocument(docItem->document());
    if (treeItemDoc != nullptr) {
        treeItemDoc->addChild(treeDocItem);
        treeItemDoc->setExpanded(true);
    }
}

void WidgetApplicationTree::onDocumentItemPropertyChanged(
        const DocumentItem *docItem, const Property *prop)
{
    QTreeWidgetItem* treeItemDocItem = this->findTreeItemDocumentItem(docItem);
    if (treeItemDocItem != nullptr) {
        if (prop == &docItem->propertyLabel)
            treeItemDocItem->setText(0, Internal::documentItemLabel(docItem));
    }
}

void WidgetApplicationTree::onTreeWidgetDocumentSelectionChanged()
{
    emit selectionChanged();
}

WidgetApplicationTree::Item::Item(Document *doc)
    : m_doc(doc),
      m_docItem(nullptr),
      m_xdeDocLabel(XdeDocumentItem::Label::null())
{ }

WidgetApplicationTree::Item::Item(DocumentItem *docItem)
    : m_doc(nullptr),
      m_docItem(docItem),
      m_xdeDocLabel(XdeDocumentItem::Label::null())
{ }

WidgetApplicationTree::Item::Item(const XdeDocumentItem::Label &lbl)
    : m_doc(nullptr),
      m_docItem(nullptr),
      m_xdeDocLabel(lbl)
{ }

bool WidgetApplicationTree::Item::isValid() const {
    return m_docItem != nullptr && !m_xdeDocLabel.label.IsNull();
}

bool WidgetApplicationTree::Item::isDocument() const
{
    return m_doc != nullptr;
}

bool WidgetApplicationTree::Item::isDocumentItem() const {
    return m_docItem != nullptr;
}

bool WidgetApplicationTree::Item::isXdeDocumentItemLabel() const {
    return !m_xdeDocLabel.label.IsNull();
}

Document *WidgetApplicationTree::Item::document() const
{
    if (this->isDocument())
        return m_doc;
    else if (this->isDocumentItem())
        return m_docItem->document();
    else if (this->isXdeDocumentItemLabel())
        return m_xdeDocLabel.xdeDocumentItem->document();
    return nullptr;
}

DocumentItem *WidgetApplicationTree::Item::documentItem() const
{
    if (this->isDocumentItem())
        return m_docItem;
    else if (this->isXdeDocumentItemLabel())
        return m_xdeDocLabel.xdeDocumentItem;
    return nullptr;
}

const XdeDocumentItem::Label& WidgetApplicationTree::Item::xdeDocumentItemLabel() const
{
    return this->isXdeDocumentItemLabel() ?
                m_xdeDocLabel :
                XdeDocumentItem::Label::null();
}

} // namespace Mayo
