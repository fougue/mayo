#pragma once

#include <QtWidgets/QWidget>
#include <vector>
class QTreeWidgetItem;

namespace Mayo {

class Application;
class Document;
class DocumentItem;

class ApplicationView : public QWidget
{
    Q_OBJECT

public:
    ApplicationView(Application* app, QWidget* widget);
    ~ApplicationView();

    std::vector<DocumentItem*> selectedDocumentItems() const;

signals:
    void selectionChanged();

private:
    void onDocumentAdded(Document* doc);
    void onDocumentErased(Document* doc);
    void onDocumentItemAdded(DocumentItem* docItem);
    void onTreeWidgetDocumentSelectionChanged();

    struct TreeWidgetItem_Document {
        QTreeWidgetItem* treeItem = nullptr;
        Document* doc = nullptr;
    };
    struct TreeWidgetItem_DocumentItem {
        QTreeWidgetItem* treeItem = nullptr;
        DocumentItem* docItem = nullptr;
    };
    std::vector<TreeWidgetItem_Document>::iterator
    findTreeItemDocument(const Document* doc);

    Application* m_app = nullptr;
    class Ui_ApplicationView* m_ui = nullptr;
    std::vector<TreeWidgetItem_Document> m_vecTreeItemDoc;
    std::vector<TreeWidgetItem_DocumentItem> m_vecTreeItemDocItem;
};

} // namespace Mayo
