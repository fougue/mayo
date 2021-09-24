/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_item.h"
#include "filepath.h"
#include "io_format.h"
#include "io_reader.h"
#include "io_writer.h"
#include "property.h"
#include "span.h"
#include "text_id.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace Mayo {

class Messenger;
class TaskProgress;

namespace IO {

class ParametersProvider;

// Main class to centralize access to FactoryReader/FactoryWriter objects
// Provides also high-level import/export services
class System {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::System)
public:
    ~System() = default;

    struct FormatProbeInput {
        FilePath filepath;
        QByteArray contentsBegin; // Excerpt of the file(from start)
        uint64_t hintFullSize; // Full file size in bytes
    };
    using FormatProbe = std::function<Format (const FormatProbeInput&)>;
    void addFormatProbe(const FormatProbe& probe);
    Format probeFormat(const FilePath& filepath) const;

    void addFactoryReader(std::unique_ptr<FactoryReader> ptr);
    void addFactoryWriter(std::unique_ptr<FactoryWriter> ptr);

    const FactoryReader* findFactoryReader(Format format) const;
    const FactoryWriter* findFactoryWriter(Format format) const;

    std::unique_ptr<Reader> createReader(Format format) const;
    std::unique_ptr<Writer> createWriter(Format format) const;

    Span<const Format> readerFormats() const { return m_vecReaderFormat; }
    Span<const Format> writerFormats() const { return m_vecWriterFormat; }

    // Import service

    struct Args_ImportInDocument {
        DocumentPtr targetDocument;
        Span<const FilePath> filepaths; // The files to be imported in target document
        const ParametersProvider* parametersProvider = nullptr;
        std::function<void(TDF_Label, TaskProgress*)> entityPostProcess;
        std::function<bool(Format)> entityPostProcessRequiredIf;
        int entityPostProcessProgressSize = 0;
        std::string entityPostProcessProgressStep;
        Messenger* messenger = nullptr;
        TaskProgress* progress = nullptr;
    };
    bool importInDocument(const Args_ImportInDocument& args);

    // Export service

    struct Args_ExportApplicationItems {
        Span<const ApplicationItem> applicationItems;
        FilePath targetFilepath;
        Format targetFormat = Format_Unknown;
        const PropertyGroup* parameters = nullptr;
        Messenger* messenger = nullptr;
        TaskProgress* progress = nullptr;
    };
    bool exportApplicationItems(const Args_ExportApplicationItems& args);

    // Fluent API: import service

    struct Operation_ImportInDocument {
        using Operation = Operation_ImportInDocument;
        Operation& targetDocument(const DocumentPtr& document);
        Operation& withFilepath(const FilePath& filepath);
        Operation& withFilepaths(Span<const FilePath> filepaths);
        Operation& withParametersProvider(const ParametersProvider* provider);

        // Post-processing executed before adding entities into Document
        Operation& withEntityPostProcess(std::function<void(TDF_Label, TaskProgress*)> fn);
        Operation& withEntityPostProcessRequiredIf(std::function<bool(Format)> fn);
        Operation& withEntityPostProcessInfoProgress(int progressSize, std::string_view progressStep);

        Operation& withMessenger(Messenger* messenger);
        Operation& withTaskProgress(TaskProgress* progress);
        bool execute();

    private:
        friend class System;
        Operation_ImportInDocument(System& system);
        System& m_system;
        Args_ImportInDocument m_args;
    };
    Operation_ImportInDocument importInDocument();

    // Fluent API: export service

    struct Operation_ExportApplicationItems {
        using Operation = Operation_ExportApplicationItems;
        Operation& targetFile(const FilePath& filepath);
        Operation& targetFormat(Format format);
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
    Operation_ExportApplicationItems exportApplicationItems();

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
