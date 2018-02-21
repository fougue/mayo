/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#ifdef HAVE_GMIO
#  include <gmio_core/text_format.h>
#  include <gmio_stl/stl_format.h>
#endif
#include <QtCore/QObject>
#include <string>
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

    // -- API
    static Application* instance();

    const std::vector<Document*>& documents() const;

    Document* addDocument(const QString& label = QString());
    bool eraseDocument(Document* doc);

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
