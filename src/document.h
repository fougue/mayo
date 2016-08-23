#pragma once

#include <QtCore/QObject>
#include <atomic>
#include <vector>

namespace qttask { class Progress; }

namespace Mayo {

class DocumentItem;
class PartItem;

class Document : public QObject
{
    Q_OBJECT

public:
    enum class PartFormat
    {
        Unknown,
        Iges,
        Step,
        OccBrep,
        Stl
    };

    Document(QObject* parent = nullptr);
    ~Document();

    bool importIges(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStep(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importOccBRep(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool importStl(
            const QString& filepath, qttask::Progress* progress = nullptr);
    bool import(
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);

    static const std::vector<PartFormat>& partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();

    bool isEmpty() const;

signals:
    void partImported(const PartItem* partItem);

private:
    void addPartItem(PartItem* part);

    std::vector<DocumentItem*> m_rootDocumentItems;
};

} // namespace Mayo
