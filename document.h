#pragma once

#include "part.h"

#include <QtCore/QObject>
#include <atomic>
#include <vector>

class QMutex;
namespace qttask { class Progress; }

namespace Mayo {

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

    static void qtRegisterRequiredMetaTypes();

signals:
    void partImported(uint64_t partId, const Part& part);

private:
    void addPart(const Part& part);

    struct Id_Part {
        uint64_t id;
        Part part;
    };
    std::vector<Id_Part> m_vecIdPart;
    std::atomic<uint64_t> m_seqPartId = 0;
};

} // namespace Mayo
