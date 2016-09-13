#pragma once

#include <QtWidgets/QWidget>
#include <vector>
class QTreeWidgetItem;

namespace Mayo {

class Document;
class DocumentItem;
class Property;

class ApplicationTreeWidget : public QWidget
{
    Q_OBJECT

public:
    ApplicationTreeWidget(QWidget* widget = nullptr);
    ~ApplicationTreeWidget();

    std::vector<DocumentItem*> selectedDocumentItems() const;

signals:
    void selectionChanged();

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(const Document* doc);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onDocumentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

    void onTreeWidgetDocumentSelectionChanged();

    struct TreeWidgetItem_Document {
        QTreeWidgetItem* treeItem;
        Document* doc;
    };
    struct TreeWidgetItem_DocumentItem {
        QTreeWidgetItem* treeItem;
        DocumentItem* docItem;
    };
    std::vector<TreeWidgetItem_Document>::iterator
    findTreeItemDocument(const Document* doc);
    std::vector<TreeWidgetItem_DocumentItem>::iterator
    findTreeItemDocumentItem(const DocumentItem* docItem);

    class Ui_ApplicationTreeWidget* m_ui = nullptr;
    std::vector<TreeWidgetItem_Document> m_vecTreeItemDoc;
    std::vector<TreeWidgetItem_DocumentItem> m_vecTreeItemDocItem;
};

} // namespace Mayo
