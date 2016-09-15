#pragma once

#include <QtCore/QObject>
#include <vector>

namespace qttask { class Progress; }

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

    enum class PartFormat
    {
        Unknown,
        Iges,
        Step,
        OccBrep,
        Stl
    };
    static const std::vector<PartFormat>& partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();

    bool importInDocument(
            Document* doc,
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);

signals:
    void documentAdded(Document* doc);
    void documentErased(const Document* doc);
    void documentItemAdded(DocumentItem* docItem);
    void documentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

private:
    Application(QObject* parent = nullptr);

    bool importIges(
            Document* doc, const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStep(
            Document* doc, const QString& filepath, qttask::Progress* progress = nullptr);
    bool importOccBRep(
            Document* doc, const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStl(
            Document* doc, const QString& filepath, qttask::Progress* progress = nullptr);

    std::vector<Document*> m_documents;
};

} // namespace Mayo
