#include "application_tree_widget.h"

#include "application.h"
#include "document.h"
#include "document_item.h"
#include "ui_application_tree_widget.h"

namespace Mayo {

namespace Internal {

static QString documentItemLabel(const DocumentItem* docItem)
{
    const QString docItemLabel = docItem->propertyLabel.value();
    return !docItemLabel.isEmpty() ?
                docItemLabel :
                ApplicationTreeWidget::tr("<unnamed>");
}

} // namespace Internal

ApplicationTreeWidget::ApplicationTreeWidget(QWidget *widget)
    : QWidget(widget),
      m_ui(new Ui_ApplicationTreeWidget)
{
    m_ui->setupUi(this);

    auto app = Application::instance();
    QObject::connect(
                app, &Application::documentAdded,
                this, &ApplicationTreeWidget::onDocumentAdded);
    QObject::connect(
                app, &Application::documentErased,
                this, &ApplicationTreeWidget::onDocumentErased);
    QObject::connect(
                app, &Application::documentItemAdded,
                this, &ApplicationTreeWidget::onDocumentItemAdded);
    QObject::connect(
                app, &Application::documentItemPropertyChanged,
                this, &ApplicationTreeWidget::onDocumentItemPropertyChanged);
    QObject::connect(
                m_ui->treeWidget_App->selectionModel(),
                &QItemSelectionModel::selectionChanged,
                this,
                &ApplicationTreeWidget::onTreeWidgetDocumentSelectionChanged);
}

ApplicationTreeWidget::~ApplicationTreeWidget()
{
    delete m_ui;
}

std::vector<DocumentItem*> ApplicationTreeWidget::selectedDocumentItems() const
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

void ApplicationTreeWidget::onDocumentAdded(Document *doc)
{
    auto treeItem = new QTreeWidgetItem;
    const QString docLabel =
            !doc->label().isEmpty() ? doc->label() : tr("<unnamed>");
    treeItem->setText(0, docLabel);
    treeItem->setIcon(0, QPixmap(":/images/document.png"));
    const TreeWidgetItem_Document pair = { treeItem, doc };
    m_vecTreeItemDoc.emplace_back(std::move(pair));
    m_ui->treeWidget_App->addTopLevelItem(treeItem);
}

void ApplicationTreeWidget::onDocumentErased(const Document *doc)
{
    auto itFound = this->findTreeItemDocument(doc);
    if (itFound != m_vecTreeItemDoc.end()) {
        delete itFound->treeItem;
        m_vecTreeItemDoc.erase(itFound);
    }
}

void ApplicationTreeWidget::onDocumentItemAdded(DocumentItem *docItem)
{
    auto treeItem = new QTreeWidgetItem;
    const QString docItemLabel = Internal::documentItemLabel(docItem);
    treeItem->setText(0, docItemLabel);
    //treeItem->setIcon(0, QPixmap(":/images/document.png"));
    const TreeWidgetItem_DocumentItem pair = { treeItem, docItem };
    m_vecTreeItemDocItem.emplace_back(std::move(pair));
    auto itFound = this->findTreeItemDocument(docItem->document());
    if (itFound != m_vecTreeItemDoc.end()) {
        itFound->treeItem->addChild(treeItem);
        itFound->treeItem->setExpanded(true);
    }
}

void ApplicationTreeWidget::onDocumentItemPropertyChanged(
        const DocumentItem *docItem, const Property *prop)
{
    auto itFound = this->findTreeItemDocumentItem(docItem);
    if (itFound != m_vecTreeItemDocItem.end()) {
        TreeWidgetItem_DocumentItem pair = *itFound;
        if (prop == &docItem->propertyLabel)
            pair.treeItem->setText(0, Internal::documentItemLabel(docItem));
    }
}

void ApplicationTreeWidget::onTreeWidgetDocumentSelectionChanged()
{
    emit selectionChanged();
}

std::vector<ApplicationTreeWidget::TreeWidgetItem_Document>::iterator
ApplicationTreeWidget::findTreeItemDocument(const Document* doc)
{
    auto itFound = std::find_if(
                m_vecTreeItemDoc.begin(),
                m_vecTreeItemDoc.end(),
                [=](const TreeWidgetItem_Document& pair)
                { return pair.doc == doc; });
    return itFound;
}

std::vector<ApplicationTreeWidget::TreeWidgetItem_DocumentItem>::iterator
ApplicationTreeWidget::findTreeItemDocumentItem(const DocumentItem *docItem)
{
    auto itFound = std::find_if(
                m_vecTreeItemDocItem.begin(),
                m_vecTreeItemDocItem.end(),
                [=](const TreeWidgetItem_DocumentItem& pair)
                { return pair.docItem == docItem; });
    return itFound;
}

} // namespace Mayo
