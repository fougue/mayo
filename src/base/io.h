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

#include "property.h"

namespace Mayo {

class TaskProgress;

namespace IO {

struct Format {
    QByteArray identifier;
    QString name;
    QStringList fileSuffixes;
};

bool operator==(const Format& lhs, const Format& rhs) {
    return lhs.identifier.compare(rhs.identifier, Qt::CaseInsensitive) == 0;
}

bool operator!=(const Format& lhs, const Format& rhs) {
    return lhs.identifier.compare(rhs.identifier, Qt::CaseInsensitive) != 0;
}

const Format Format_Unknown = { "", "Format_Unknown", {} };
const Format Format_STEP = { "STEP", "STEP(ISO 10303)", { "stp", "step" } };
const Format Format_IGES = { "IGES", "IGES(ASME Y14.26M))", { "igs", "iges" } };
const Format Format_OCCBREP = { "OCCBREP", "OpenCascade BREP", { "brep", "rle", "occ" } };
const Format Format_STL = { "STL", "STL(STereo-Lithography)", { "stl" } };
const Format Format_OBJ = { "OBJ", "Wavefront OBJ", { "obj" } };
const Format Format_GLTF = { "GLTF", "glTF(GL Transmission Format)", { "gltf", "glb" } };
const Format Format_VRML = { "VRML", "VRML(ISO/CEI 14772-2)", { "wrl", "wrz", "vrml" } };

class Reader {
public:
    virtual bool readFile(const QString& filepath, TaskProgress* progress) = 0;
    virtual bool transfer(DocumentPtr doc, TaskProgress* progress) = 0;
};

class Writer {
public:
    virtual bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) = 0;
    virtual bool writeFile(const QString& filepath, TaskProgress* progress) = 0;
};

class FactoryReader {
public:
    virtual Span<const Format> formats() const = 0;
    virtual Format findFormatFromContents(
            QByteArray contentsBegin,
            uint64_t hintFullContentsSize) const = 0;

    virtual std::unique_ptr<Reader> create(const Format& format) const = 0;    
    virtual std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
    virtual bool applyParameters(const PropertyGroup& params, Reader* reader) const = 0;
};

class FactoryWriter {
public:
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Writer> create(const Format& format) const = 0;
    virtual std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
    virtual bool applyParameters(const PropertyGroup& params, Writer* writer) const = 0;
};

class System {
    Q_DECLARE_TR_FUNCTIONS(Mayo::IO::System)
public:
    void addFactoryReader(std::unique_ptr<FactoryReader> ptr);
    void addFactoryWriter(std::unique_ptr<FactoryWriter> ptr);

    const FactoryReader* findFactoryReader(const Format& format) const;
    const FactoryWriter* findFactoryWriter(const Format& format) const;

    std::unique_ptr<Reader> createReader(const Format& format) const;
    std::unique_ptr<Writer> createWriter(const Format& format) const;

    Span<const Format> readerFormats() const { return m_vecReaderFormat; }
    Span<const Format> writerFormats() const { return m_vecWriterFormat; }
    Format findFileFormat(const QString& filepath) const;
    static QString fileFilter(const Format& format);

    // BASE

    struct Args_BaseImportExport {
        const PropertyGroup* parameters = nullptr;
        Messenger* messenger = nullptr;
        TaskProgress* progress = nullptr;
    };

    // IMPORT

    struct Args_ImportInDocument : public Args_BaseImportExport {
        DocumentPtr targetDocument;
        QStringList filepaths;
    };

    struct Operation_ImportInDocument {
        using Operation = Operation_ImportInDocument;
        Operation& targetDocument(const DocumentPtr& document);
        Operation& withFilepaths(const QStringList& filepaths);
        Operation& withParameters(const PropertyGroup& parameters);
        Operation& withMessenger(Messenger* messenger);
        Operation& withTaskProgress(TaskProgress* progress);
        bool execute();

    private:
        friend class System;
        Operation_ImportInDocument(System& system);
        System& m_system;
        Args_ImportInDocument m_args;
    };

    bool importInDocument(const Args_ImportInDocument& args);
    Operation_ImportInDocument importInDocument(); // Fluent API

    // EXPORT

    struct Args_ExportApplicationItems : public Args_BaseImportExport {
        Span<const ApplicationItem> applicationItems;
        QString targetFilepath;
        Format targetFormat = Format_Unknown;
    };

    struct Operation_ExportApplicationItems {
        using Operation = Operation_ExportApplicationItems;
        Operation& targetFile(const QString& filepath);
        Operation& targetFormat(const Format& format);
        Operation& withItems(Span<const ApplicationItem> appItems);
        Operation& withParameters(const PropertyGroup& parameters);
        Operation& withMessenger(Messenger* messenger);
        Operation& withTaskProgress(TaskProgress* progress);
        bool execute();

    private:
        friend class System;
        Operation_ExportApplicationItems(System& system);
        System& m_system;
        Args_ExportApplicationItems m_args;
    };

    bool exportApplicationItems(const Args_ExportApplicationItems& args);
    Operation_ExportApplicationItems exportApplicationItems(); // Fluent API

    // Implementation
private:
    std::vector<Format> m_vecReaderFormat;
    std::vector<Format> m_vecWriterFormat;
    std::vector<std::unique_ptr<FactoryReader>> m_vecFactoryReader;
    std::vector<std::unique_ptr<FactoryWriter>> m_vecFactoryWriter;
};

} // namespace IO

#if 0
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

    using Result = Mayo::Result<void>;

    struct ExportOptions {
#ifdef HAVE_GMIO
        gmio_stl_format stlFormat = GMIO_STL_FORMAT_UNKNOWN;
        std::string stlaSolidName;
        gmio_float_text_format stlaFloat32Format = GMIO_FLOAT_TEXT_FORMAT_SHORTEST_LOWERCASE;
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
            TaskProgress* progress = nullptr);
    IO::Result exportApplicationItems(
            Span<const ApplicationItem> appItems,
            PartFormat format,
            const ExportOptions& options,
            const QString& filepath,
            TaskProgress* progress = nullptr);
    static bool hasExportOptionsForFormat(PartFormat format);

private:
    IO();

    struct ImportData {
        DocumentPtr doc;
        QString filepath;
        TaskProgress* progress;
    };

    struct ExportData {
        Span<const ApplicationItem> appItems;
        ExportOptions options;
        QString filepath;
        TaskProgress* progress;
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
#endif

} // namespace Mayo
