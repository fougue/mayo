/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "property.h"
#include "xde_document_item.h"

#include <QtWidgets/QWidget>
class QItemSelection;
class QTreeWidgetItem;

namespace Mayo {

class WidgetApplicationTree : public QWidget {
    Q_OBJECT
public:
    using Item = ApplicationItem;

    WidgetApplicationTree(QWidget* widget = nullptr);
    ~WidgetApplicationTree();

    bool isMergeXdeReferredShapeOn() const;
    void setMergeXdeReferredShape(bool on);

    const QString& referenceItemTextTemplate() const;
    void setReferenceItemTextTemplate(const QString& textTemplate);

    void refreshItemText(const ApplicationItem& appItem);

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);
    void onDocumentPropertyChanged(Document* doc, Property* prop);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onDocumentItemErased(const DocumentItem* docItem);
    void onDocumentItemPropertyChanged(DocumentItem* docItem, Property* prop);

    void onTreeWidgetDocumentSelectionChanged(
            const QItemSelection &selected, const QItemSelection &deselected);

    QTreeWidgetItem* loadDocumentItem(DocumentItem* docItem);
    void guiBuildXdeTree(
            QTreeWidgetItem* treeDocItem, XdeDocumentItem* xdeDocItem);

    QTreeWidgetItem* findTreeItemDocument(const Document* doc) const;
    QTreeWidgetItem* findTreeItemDocumentItem(const DocumentItem* docItem) const;
    QTreeWidgetItem* findTreeItemXdeLabel(const DocumentItem* docItem, const TDF_Label& label) const;

    QString referenceItemText(
            const TDF_Label& refLabel,
            const TDF_Label& referredLabel) const;

    void refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item);

    class Ui_WidgetApplicationTree* m_ui = nullptr;
    bool m_isMergeXdeReferredShapeOn = true;
    QString m_refItemTextTemplate;
};

} // namespace Mayo
