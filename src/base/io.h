/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "io_format.h"
#include "messenger.h"
#include "property.h"
#include "result.h"
#include "span.h"

#include <QtCore/QCoreApplication>
#include <functional>
#include <memory>

namespace Mayo { class TaskProgress; }

namespace Mayo {
namespace IO {

class Reader {
public:
    virtual bool readFile(const QString& filepath, TaskProgress* progress) = 0;
    virtual bool transfer(DocumentPtr doc, TaskProgress* progress) = 0;
    virtual void applyParameters(const PropertyGroup* /*params*/) {}
};

class Writer {
public:
    virtual bool transfer(Span<const ApplicationItem> appItems, TaskProgress* progress) = 0;
    virtual bool writeFile(const QString& filepath, TaskProgress* progress) = 0;
    virtual void applyParameters(const PropertyGroup* /*params*/) {}
};

class FactoryReader {
public:
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Reader> create(const Format& format) const = 0;    
    virtual std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
};

class FactoryWriter {
public:
    virtual Span<const Format> formats() const = 0;
    virtual std::unique_ptr<Writer> create(const Format& format) const = 0;
    virtual std::unique_ptr<PropertyGroup> createParameters(
            const Format& format,
            PropertyGroup* parentGroup) const = 0;
};

class ParametersProvider {
public:
    virtual const PropertyGroup* findReaderParameters(const Format& format) const = 0;
    virtual const PropertyGroup* findWriterParameters(const Format& format) const = 0;
};

class System {
    Q_DECLARE_TR_FUNCTIONS(Mayo::IO::System)
public:
    struct FormatProbeInput {
        QString filepath;
        QByteArray contentsBegin;
        uint64_t hintFullSize;
    };
    using FormatProbe = std::function<Format (const FormatProbeInput&)>;
    void addFormatProbe(const FormatProbe& probe);
    Format probeFormat(const QString& filepath) const;

    void addFactoryReader(std::unique_ptr<FactoryReader> ptr);
    void addFactoryWriter(std::unique_ptr<FactoryWriter> ptr);

    const FactoryReader* findFactoryReader(const Format& format) const;
    const FactoryWriter* findFactoryWriter(const Format& format) const;

    std::unique_ptr<Reader> createReader(const Format& format) const;
    std::unique_ptr<Writer> createWriter(const Format& format) const;

    Span<const Format> readerFormats() const { return m_vecReaderFormat; }
    Span<const Format> writerFormats() const { return m_vecWriterFormat; }
    static QString fileFilter(const Format& format);

    // BASE

    struct Args_BaseImportExport {
        Messenger* messenger = nullptr;
        TaskProgress* progress = nullptr;
    };

    // IMPORT

    struct Args_ImportInDocument : public Args_BaseImportExport {
        DocumentPtr targetDocument;
        QStringList filepaths;
        const ParametersProvider* parametersProvider = nullptr;
    };

    struct Operation_ImportInDocument {
        using Operation = Operation_ImportInDocument;
        Operation& targetDocument(const DocumentPtr& document);
        Operation& withFilepath(const QString& filepath);
        Operation& withFilepaths(const QStringList& filepaths);
        Operation& withParametersProvider(const ParametersProvider* provider);
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
        const PropertyGroup* parameters = nullptr;
    };

    struct Operation_ExportApplicationItems {
        using Operation = Operation_ExportApplicationItems;
        Operation& targetFile(const QString& filepath);
        Operation& targetFormat(const Format& format);
        Operation& withItems(Span<const ApplicationItem> appItems);
        Operation& withParameters(const PropertyGroup* parameters);
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
    std::vector<FormatProbe> m_vecFormatProbe;
    std::vector<Format> m_vecReaderFormat;
    std::vector<Format> m_vecWriterFormat;
    std::vector<std::unique_ptr<FactoryReader>> m_vecFactoryReader;
    std::vector<std::unique_ptr<FactoryWriter>> m_vecFactoryWriter;
};

// Predefined
Format probeFormat_STEP(const System::FormatProbeInput& input);
Format probeFormat_IGES(const System::FormatProbeInput& input);
Format probeFormat_OCCBREP(const System::FormatProbeInput& input);
Format probeFormat_STL(const System::FormatProbeInput& input);
Format probeFormat_OBJ(const System::FormatProbeInput& input);
void addPredefinedFormatProbes(System* system);

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
