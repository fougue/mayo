#include "application_view.h"

#include "application.h"
#include "document.h"
#include "document_item.h"
#include "ui_application_view.h"

namespace Mayo {

ApplicationView::ApplicationView(Application* app, QWidget *widget)
    : QWidget(widget),
      m_app(app),
      m_ui(new Ui_ApplicationView)
{
    m_ui->setupUi(this);

    QObject::connect(
                app, &Application::documentAdded,
                this, &ApplicationView::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &ApplicationView::onDocumentErased);
    QObject::connect(
                app, &Application::documentItemAdded,
                this, &ApplicationView::onDocumentItemAdded);
    QObject::connect(
                m_ui->treeWidget_App->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &ApplicationView::onTreeWidgetDocumentSelectionChanged);
}

ApplicationView::~ApplicationView()
{
    delete m_ui;
}

std::vector<DocumentItem*> ApplicationView::selectedDocumentItems() const
{
    const QList<QTreeWidgetItem*> listTreeItem =
            m_ui->treeWidget_App->selectedItems();
    std::vector<DocumentItem*> vecDocItem;
    vecDocItem.reserve(listTreeItem.size());
    for (QTreeWidgetItem* treeItem : listTreeItem) {
        auto itFound = std::find_if(
                    m_vecTreeItemDocItem.cbegin(),
                    m_vecTreeItemDocItem.cend(),
                    [=](const TreeWidgetItem_DocumentItem& pair) {
            return pair.treeItem == treeItem;
        });
        if (itFound != m_vecTreeItemDocItem.cend())
            vecDocItem.push_back(itFound->docItem);
    }
    return vecDocItem;
}

void ApplicationView::onDocumentAdded(Document *doc)
{
    auto treeItem = new QTreeWidgetItem;
    const QString docLabel =
            !doc->label().isEmpty() ? doc->label() : tr("<unnamed>");
    treeItem->setText(0, docLabel);
    treeItem->setIcon(0, QPixmap(":/images/document.png"));
    TreeWidgetItem_Document pair;
    pair.treeItem = treeItem;
    pair.doc = doc;
    m_vecTreeItemDoc.emplace_back(std::move(pair));
    m_ui->treeWidget_App->addTopLevelItem(treeItem);
}

void ApplicationView::onDocumentErased(Document *doc)
{
    auto itFound = this->findTreeItemDocument(doc);
    if (itFound != m_vecTreeItemDoc.end()) {
        delete itFound->treeItem;
        m_vecTreeItemDoc.erase(itFound);
    }
}

void ApplicationView::onDocumentItemAdded(DocumentItem *docItem)
{
    auto treeItem = new QTreeWidgetItem;
    const QString itemLabel =
            !docItem->label().isEmpty() ? docItem->label() : tr("<unnamed>");
    treeItem->setText(0, itemLabel);
    //treeItem->setIcon(0, QPixmap(":/images/document.png"));
    TreeWidgetItem_DocumentItem pair;
    pair.treeItem = treeItem;
    pair.docItem = docItem;
    m_vecTreeItemDocItem.emplace_back(std::move(pair));
    auto itFound = this->findTreeItemDocument(docItem->document());
    if (itFound != m_vecTreeItemDoc.end())
        itFound->treeItem->addChild(treeItem);
}

void ApplicationView::onTreeWidgetDocumentSelectionChanged()
{
    emit selectionChanged();
}

std::vector<ApplicationView::TreeWidgetItem_Document>::iterator
ApplicationView::findTreeItemDocument(const Document* doc)
{
    auto itFound = std::find_if(
                m_vecTreeItemDoc.begin(),
                m_vecTreeItemDoc.end(),
                [=](const TreeWidgetItem_Document& pair)
                { return pair.doc == doc; });
    return itFound;
}

} // namespace Mayo
