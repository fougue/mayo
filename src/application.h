/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

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

    struct IoResult {
        bool ok;
        QString errorText;
        operator bool() const { return ok; }
    };

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

    using ArrayDocument = std::vector<Document*>;
    using ArrayDocumentConstIterator = ArrayDocument::const_iterator;

    // -- API
    static Application* instance();

    const ArrayDocument& documents() const;
    Document* documentAt(int index) const;

    Document* createDocument(const QString& label = QString());
    void addDocument(Document* doc);
    bool eraseDocument(Document* doc);

    ArrayDocumentConstIterator findDocumentByLocation(const QFileInfo& loc) const;

    static const std::vector<PartFormat>& partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();
    static PartFormat findPartFormat(const QString& filepath);

    IoResult importInDocument(
            Document* doc,
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    IoResult exportDocumentItems(
            const std::vector<DocumentItem*>& docItems,
            PartFormat format,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    static bool hasExportOptionsForFormat(PartFormat format);

signals:
    void documentAdded(Document* doc);
    void documentErased(const Document* doc);
    void documentItemAdded(DocumentItem* docItem);
    void documentItemPropertyChanged(
            const DocumentItem* docItem, const Property* prop);

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
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStep(
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportOccBRep(
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl(
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl_gmio(
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);
    IoResult exportStl_OCC(
            const std::vector<DocumentItem*>& docItems,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress);

    std::vector<Document*> m_documents;
};

} // namespace Mayo
