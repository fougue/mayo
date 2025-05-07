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
#include "libtree.h"
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
        std::string_view contentsBegin; // Excerpt of the file(from start)
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

    //
    // Import service
    //

    // Contains arguments for the importInDocument() function
    struct Args_ImportInDocument {
        // Target document where entities read from `filepaths` will be imported
        DocumentPtr targetDocument;

        // List of files to be imported in target document
        Span<const FilePath> filepaths;

        // Optional: provider of format-specific parameters to be considered when reading
        const ParametersProvider* parametersProvider = nullptr;

        // Optional: function applied to each imported entity. Executed before adding entities into
        // target document
        //     1st arg: CAF label of the entity to "post-process"
        //     2nd arg: progress indicator of the post-process function
        std::function<void(TDF_Label, TaskProgress*)> entityPostProcess;

        // Optional: predicate telling whether imported entities have to be post-processed(ie whether
        //           `entityPostProcess` function has to be called)
        // The single argument being the format of the file from which entities were read
        std::function<bool(Format)> entityPostProcessRequiredIf;

        // Optional: progress size(eg 25%) of the whole post-process operation
        int entityPostProcessProgressSize = 0;

        // Optional: title of the whole post-process operation
        std::string entityPostProcessProgressStep;

        // Optional: the messenger object used to report any additional infos, warnings and errors
        Messenger* messenger = nullptr;

        // Optional: the indicator object used to report progress of the import operation
        TaskProgress* progress = nullptr;
    };
    bool importInDocument(const Args_ImportInDocument& args) const;

    //
    // Export service
    //

    // Contains arguments for the exportApplicationItems() function
    struct Args_ExportApplicationItems {
        // List of items to be exported
        Span<const ApplicationItem> applicationItems;

        // Path to the target file where items will be written
        FilePath targetFilepath;

        // Format in which items are exported
        Format targetFormat = Format_Unknown;

        // Optional: format-specific parameters to be considered when writing items
        const PropertyGroup* parameters = nullptr; // TODO use ParametersProvider instead?

        // Optional: the messenger object used to report any additional infos, warnings and errors
        Messenger* messenger = nullptr;

        // Optional: the indicator object used to report progress of the import operation
        TaskProgress* progress = nullptr;
    };
    bool exportApplicationItems(const Args_ExportApplicationItems& args) const;

    //
    // Fluent API: import service
    //

    // Helper struct to provide fluent-like interface over Args_ImportInDocument
    // See also https://en.wikipedia.org/wiki/Fluent_interface
    struct Operation_ImportInDocument {
        using Operation = Operation_ImportInDocument;
        Operation& targetDocument(const DocumentPtr& document);
        Operation& withFilepath(const FilePath& filepath);
        Operation& withFilepaths(Span<const FilePath> filepaths);
        Operation& withParametersProvider(const ParametersProvider* provider);

        Operation& withEntityPostProcess(std::function<void(TDF_Label, TaskProgress*)> fn);
        Operation& withEntityPostProcessRequiredIf(std::function<bool(Format)> fn);
        Operation& withEntityPostProcessInfoProgress(int progressSize, std::string_view progressStep);

        Operation& withMessenger(Messenger* messenger);
        Operation& withTaskProgress(TaskProgress* progress);
        bool execute(); // Runs System::importInDocument() function

    private:
        friend class System;
        Operation_ImportInDocument(const System& system);
        const System& m_system;
        Args_ImportInDocument m_args;
    };
    Operation_ImportInDocument importInDocument() const;

    //
    // Fluent API: export service
    //

    // Helper struct to provide fluent-like interface over Args_ExportApplicationItems
    // See also https://en.wikipedia.org/wiki/Fluent_interface
    struct Operation_ExportApplicationItems {
        using Operation = Operation_ExportApplicationItems;
        Operation& targetFile(const FilePath& filepath);
        Operation& targetFormat(Format format);
        Operation& withItem(const ApplicationItem& appItem);
        Operation& withItems(Span<const ApplicationItem> appItems);
        Operation& withParameters(const PropertyGroup* parameters);
        Operation& withMessenger(Messenger* messenger);
        Operation& withTaskProgress(TaskProgress* progress);
        bool execute(); // Runs System::exportApplicationItems() function

    private:
        friend class System;
        Operation_ExportApplicationItems(const System& system);
        const System& m_system;
        Args_ExportApplicationItems m_args;
    };
    Operation_ExportApplicationItems exportApplicationItems() const;

    // Helpers

    // Iterate over `spanItem` and call `fnCallback` for each item. Guarantees that doublon items
    // will be visited only once
    static void visitUniqueItems(
            Span<const ApplicationItem> spanItem,
            std::function<void(const ApplicationItem&)> fnCallback
    );

    // Iterate over `spanItem` and then deep traverse the corresponding tree node to
    // call `fnCallback` for each item. Guarantees that doublon items will be visited only once
    static void traverseUniqueItems(
            Span<const ApplicationItem> spanItem,
            std::function<void(const DocumentTreeNode&)> fnCallback,
            TreeTraversal mode = TreeTraversal::PreOrder
    );

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
Format probeFormat_PLY(const System::FormatProbeInput& input);
Format probeFormat_OFF(const System::FormatProbeInput& input);
void addPredefinedFormatProbes(System* system);

} // namespace IO
} // namespace Mayo
