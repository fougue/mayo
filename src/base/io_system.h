/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "io_format.h"
#include "io_reader.h"
#include "io_writer.h"
#include "property.h"
#include "span.h"

#include <QtCore/QCoreApplication>
#include <functional>
#include <memory>

namespace Mayo {
class Messenger;
class TaskProgress;
}

namespace Mayo {
namespace IO {

class ParametersProvider;

class System {
    Q_DECLARE_TR_FUNCTIONS(Mayo::IO::System)
public:
    ~System() = default;

    struct FormatProbeInput {
        QString filepath;
        QByteArray contentsBegin; // Excerpt of the file(from start)
        uint64_t hintFullSize; // Full file size in bytes
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
} // namespace Mayo
