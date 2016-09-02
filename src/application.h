#pragma once

#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;

class Application : public QObject
{
    Q_OBJECT

public:
    static Application* instance();

    const std::vector<Document*>& documents() const;

    Document* addDocument(const QString& label = QString());
    bool eraseDocument(Document* doc);

signals:
    void documentAdded(Document* doc);
    void documentErased(Document* doc);
    void documentItemAdded(DocumentItem* docItem);

private:
    Application(QObject* parent = nullptr);

    std::vector<Document*> m_documents;
};

} // namespace Mayo
