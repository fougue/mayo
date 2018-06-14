/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "xde_document_item.h"

#include <QtWidgets/QWidget>
class QTreeWidgetItem;

namespace Mayo {

class Document;
class DocumentItem;
class XdeDocumentItem;

class WidgetApplicationTree : public QWidget {
    Q_OBJECT
public:
    struct Item {
        Item() = default;
        Item(Document* doc);
        Item(DocumentItem* docItem);
        Item(const XdeDocumentItem::Label& lbl);

        bool isValid() const;
        bool isDocument() const;
        bool isDocumentItem() const;
        bool isXdeDocumentItemLabel() const;

        Document* document() const;
        DocumentItem* documentItem() const;
        const XdeDocumentItem::Label& xdeDocumentItemLabel() const;

    private:
        Document* m_doc;
        DocumentItem* m_docItem;
        XdeDocumentItem::Label m_xdeDocLabel;
    };

    WidgetApplicationTree(QWidget* widget = nullptr);
    ~WidgetApplicationTree();

    std::vector<Item> selectedItems() const;
    bool hasSelectedDocumentItems() const;
    std::vector<DocumentItem*> selectedDocumentItems() const;

    bool isMergeXdeReferredShapeOn() const;
    void setMergeXdeReferredShape(bool on);

signals:
    void selectionChanged();

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onDocumentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

    void onTreeWidgetDocumentSelectionChanged();

    QTreeWidgetItem* loadDocumentItem(DocumentItem* docItem);
    void loadXdeShapeStructure(
            QTreeWidgetItem* treeDocItem, XdeDocumentItem* xdeDocItem);

    QTreeWidgetItem* findTreeItemDocument(const Document* doc) const;
    QTreeWidgetItem* findTreeItemDocumentItem(const DocumentItem* docItem) const;

    class Ui_WidgetApplicationTree* m_ui = nullptr;
    bool m_isMergeXdeReferredShapeOn = true;
};

} // namespace Mayo
