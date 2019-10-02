/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "result.h"
#include "span.h"

#ifdef HAVE_GMIO
#  include <gmio_core/text_format.h>
#  include <gmio_stl/stl_format.h>
#endif
#include <QtCore/QObject>
#include <string>
#include <vector>
class QFileInfo;

namespace qttask { class Progress; }

namespace Mayo {

class Document;
class DocumentItem;
class Property;

class Application : public QObject {
    Q_OBJECT
public:
    // -- Types
    enum class PartFormat {
        Unknown,
        Iges,
        Step,
        OccBrep,
        Stl
    };

    using IoResult = Result<void>;

    struct ExportOptions {
#ifdef HAVE_GMIO
        gmio_stl_format stlFormat = GMIO_STL_FORMAT_UNKNOWN;
        std::string stlaSolidName;
        gmio_float_text_format stlaFloat32Format =
                GMIO_FLOAT_TEXT_FORMAT_SHORTEST_LOWERCASE;
        uint8_t stlaFloat32Precision = 9;
#else
        enum class StlFormat {
            Ascii,
            Binary
        };
        StlFormat stlFormat = StlFormat::Binary;
#endif
    };

    // -- API
    static Application* instance();

    Span<Document* const> documents() const;
    int documentCount() const;
    Document* documentAt(int index) const;
    int findDocumentByLocation(const QFileInfo& loc) const;
    int indexOfDocument(const Document* doc) const;

    void addDocument(Document* doc);
    bool eraseDocument(Document* doc);

    static Span<const PartFormat> partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();
    static PartFormat findPartFormat(const QString& filepath);

    IoResult importInDocument(
            Document* doc,
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    IoResult exportApplicationItems(
            Span<const ApplicationItem> appItems,
            PartFormat format,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    static bool hasExportOptionsForFormat(PartFormat format);

signals:
    void documentAdded(Document* doc);
    void documentErased(const Document* doc);
    void documentPropertyChanged(Document* doc, Property* prop);
    void documentItemAdded(DocumentItem* docItem);
    void documentItemErased(const DocumentItem* docItem);
    void documentItemPropertyChanged(DocumentItem* docItem, Property* prop);

    // -- Implementation
private:
    Application(QObject* parent = nullptr);

    IoResult importIges(
            Document* doc, const QString& filepath, qttask::Progress* progress);
    IoResult importStep(
            Document* doc, const QString& filepath, qttask::Progress* progress);
    IoResult importOccBRep(
            Document* doc, const QString& filepath, qttask::Progress* progress);
    IoResult importStl(
            Document* doc, const QString& filepath, qttask::Progress* progress);

    IoResult exportIges(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStep(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportOccBRep(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl_gmio(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl_OCC(
            Span<const ApplicationItem> appItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);

    std::vector<Document*> m_documents;
};

} // namespace Mayo
