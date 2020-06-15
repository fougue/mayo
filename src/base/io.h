/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "messenger.h"
#include "result.h"
#include "span.h"
#ifdef HAVE_GMIO
#  include <gmio_core/text_format.h>
#  include <gmio_stl/stl_format.h>
#endif

#include <QtCore/QCoreApplication>
#include <memory>

namespace qttask { class Progress; }

namespace Mayo {

class Reader {
public:
    //struct Error { int code; QString text; };
    virtual bool readFile(const QString& filepath, qttask::Progress* progress = nullptr) = 0;
    virtual bool transfer(DocumentPtr doc, qttask::Progress* progress = nullptr) = 0;
};

class IO {
    Q_DECLARE_TR_FUNCTIONS(Mayo::IO)
public:
    enum class PartFormat {
        Unknown,
        Iges,
        Step,
        OccBrep,
        Stl,
        Obj
    };

    using Result = Result<void>;

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

    enum class StlIoLibrary {
        Gmio,
        OpenCascade
    };

    static IO* instance();

    static Span<const PartFormat> partFormats();
    static QString partFormatFilter(PartFormat format);
    static QStringList partFormatFilters();
    static PartFormat findPartFormat(const QString& filepath);

    StlIoLibrary stlIoLibrary() const;
    void setStlIoLibrary(StlIoLibrary lib);

    std::unique_ptr<Reader> createReader(IO::PartFormat format) const;

    bool importInDocument(
            DocumentPtr doc,
            const QStringList& listFilepath,
            Messenger* messenger = NullMessenger::instance(),
            qttask::Progress* progress = nullptr);

    IO::Result importInDocument(
            DocumentPtr doc,
            PartFormat format,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    IO::Result exportApplicationItems(
            Span<const ApplicationItem> appItems,
            PartFormat format,
            const ExportOptions& options,
            const QString& filepath,
            qttask::Progress* progress = nullptr);
    static bool hasExportOptionsForFormat(PartFormat format);

private:
    IO();

    struct ImportData {
        DocumentPtr doc;
        QString filepath;
        qttask::Progress* progress;
    };

    struct ExportData {
        Span<const ApplicationItem> appItems;
        ExportOptions options;
        QString filepath;
        qttask::Progress* progress;
    };

    Result importIges(ImportData data);
    Result importStep(ImportData data);
    Result importOccBRep(ImportData data);
    Result importStl(ImportData data);

    Result exportIges(ExportData data);
    Result exportStep(ExportData data);
    Result exportOccBRep(ExportData data);
    Result exportStl(ExportData data);
    Result exportStl_gmio(ExportData data);
    Result exportStl_OCC(ExportData data);

    StlIoLibrary m_stlIoLibrary = StlIoLibrary::OpenCascade;
};

} // namespace Mayo
