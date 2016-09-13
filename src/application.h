#pragma once

#include <QtCore/QObject>
#include <vector>

namespace Mayo {

class Document;
class DocumentItem;
class Property;

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
    void documentErased(const Document* doc);
    void documentItemAdded(DocumentItem* docItem);
    void documentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

private:
    Application(QObject* parent = nullptr);

    std::vector<Document*> m_documents;
};

} // namespace Mayo
