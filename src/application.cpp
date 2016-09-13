#include "application.h"

#include "document.h"
#include "document_item.h"

#include <algorithm>

namespace Mayo {

Application::Application(QObject *parent)
    : QObject(parent)
{
}

Application *Application::instance()
{
    static Application app;
    return &app;
}

const std::vector<Document *> &Application::documents() const
{
    return m_documents;
}

Document *Application::addDocument(const QString &label)
{
    static unsigned docSequenceId = 0;
    auto doc = new Document(this);
    if (label.isEmpty()) {
        doc->setLabel(tr("Anonymous%1")
                      .arg(docSequenceId > 0 ?
                               QString::number(docSequenceId) :
                               QString()));
    }
    QObject::connect(
                doc, &Document::itemAdded,
                this, &Application::documentItemAdded);
    QObject::connect(
                doc, &Document::itemPropertyChanged,
                this, &Application::documentItemPropertyChanged);
    m_documents.emplace_back(doc);
    ++docSequenceId;
    emit documentAdded(doc);
    return doc;
}

bool Application::eraseDocument(Document *doc)
{
    auto itFound = std::find(m_documents.cbegin(), m_documents.cend(), doc);
    if (itFound != m_documents.cend()) {
        m_documents.erase(itFound);
        delete doc;
        emit documentErased(doc);
        return true;
    }
    return false;
}

} // namespace Mayo
