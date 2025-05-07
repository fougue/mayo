/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_system.h"

#include "caf_utils.h"
#include "cpp_utils.h"
#include "document.h"
#include "io_parameters_provider.h"
#include "io_reader.h"
#include "io_writer.h"
#include "messenger.h"
#include "task_manager.h"
#include "task_progress.h"
#include "tkernel_utils.h"

#include <fmt/format.h>
#include <gsl/util>

#include <algorithm>
#include <array>
#include <fstream>
#include <locale>
#include <mutex>
#include <regex>
#include <unordered_set>
#include <vector>

namespace Mayo::IO {

namespace {

bool containsFormat(Span<const Format> spanFormat, Format format)
{
    auto itFormat = std::find(spanFormat.begin(), spanFormat.end(), format);
    return itFormat != spanFormat.end();
}

void dispatchErrors(std::string_view headerMsg, const MessageCollecter& msgCollect, Messenger* target)
{
    const std::string strErrors = msgCollect.asString("\n    ", MessageType::Error);
    if (!strErrors.empty())
        target->error() << fmt::format("{}\n    {}", headerMsg, strErrors);
}

void dispatchWarnings(std::string_view headerMsg, const MessageCollecter& msgCollect, Messenger* target)
{
    const std::string strWarnings = msgCollect.asString("\n    ", MessageType::Warning);
    if (!strWarnings.empty())
        target->warning() << fmt::format("{}\n    {}", headerMsg, strWarnings);
}

} // namespace

void System::addFormatProbe(const FormatProbe& probe)
{
    m_vecFormatProbe.push_back(probe);
}

Format System::probeFormat(const FilePath& filepath) const
{
    std::ifstream file;
    file.open(filepath);
    if (file.is_open()) {
        std::array<char, 2048> buff;
        buff.fill(0);
        file.read(buff.data(), buff.size());
        FormatProbeInput probeInput = {};
        probeInput.filepath = filepath;
        probeInput.contentsBegin = std::string_view(buff.data(), file.gcount());
        probeInput.hintFullSize = filepathFileSize(filepath);
        for (const FormatProbe& fnProbe : m_vecFormatProbe) {
            const Format format = fnProbe(probeInput);
            if (format != Format_Unknown)
                return format;
        }
    }

    // Try to guess from file suffix
    std::string fileSuffix = filepath.extension().u8string();
    if (!fileSuffix.empty() && fileSuffix.front() == '.')
        fileSuffix.erase(fileSuffix.begin());

    auto fnCharIEqual = [](char lhs, char rhs) {
        const auto& clocale = std::locale::classic();
        return std::tolower(lhs, clocale) == std::tolower(rhs, clocale);
    };
    auto fnMatchFileSuffix = [=](Format format) {
        for (std::string_view candidate : formatFileSuffixes(format)) {
            if (candidate.size() == fileSuffix.size()
                && std::equal(candidate.cbegin(), candidate.cend(), fileSuffix.cbegin(), fnCharIEqual))
            {
                return true;
            }
        }

        return false;
    };
    for (Format format : m_vecReaderFormat) {
        if (fnMatchFileSuffix(format))
            return format;
    }

    for (Format format : m_vecWriterFormat) {
        if (fnMatchFileSuffix(format))
            return format;
    }

    return Format_Unknown;
}

void System::addFactoryReader(std::unique_ptr<FactoryReader> ptr)
{
    if (!ptr)
        return;

    auto itFactory = std::find(m_vecFactoryReader.cbegin(), m_vecFactoryReader.cend(), ptr);
    if (itFactory != m_vecFactoryReader.cend())
        return;

    for (Format format : ptr->formats()) {
        auto itFormat = std::find(m_vecReaderFormat.cbegin(), m_vecReaderFormat.cend(), format);
        if (itFormat == m_vecReaderFormat.cend())
            m_vecReaderFormat.push_back(format);
    }

    m_vecFactoryReader.push_back(std::move(ptr));
}

void System::addFactoryWriter(std::unique_ptr<FactoryWriter> ptr)
{
    if (!ptr)
        return;

    auto itFactory = std::find(m_vecFactoryWriter.cbegin(), m_vecFactoryWriter.cend(), ptr);
    if (itFactory != m_vecFactoryWriter.cend())
        return;

    for (IO::Format format : ptr->formats()) {
        auto itFormat = std::find(m_vecWriterFormat.cbegin(), m_vecWriterFormat.cend(), format);
        if (itFormat == m_vecWriterFormat.cend())
            m_vecWriterFormat.push_back(format);
    }

    m_vecFactoryWriter.push_back(std::move(ptr));
}

const FactoryReader* System::findFactoryReader(Format format) const
{
    for (const std::unique_ptr<FactoryReader>& ptr : m_vecFactoryReader) {
        if (containsFormat(ptr->formats(), format))
            return ptr.get();
    }

    return nullptr;
}

const FactoryWriter* System::findFactoryWriter(Format format) const
{
    for (const std::unique_ptr<FactoryWriter>& ptr : m_vecFactoryWriter) {
        if (containsFormat(ptr->formats(), format))
            return ptr.get();
    }

    return nullptr;
}

std::unique_ptr<Reader> System::createReader(Format format) const
{
    const FactoryReader* ptr = this->findFactoryReader(format);
    if (ptr)
        return ptr->create(format);

    return {};
}

std::unique_ptr<Writer> System::createWriter(Format format) const
{
    const FactoryWriter* ptr = this->findFactoryWriter(format);
    if (ptr)
        return ptr->create(format);

    return {};
}

bool System::importInDocument(const Args_ImportInDocument& args) const
{
    DocumentPtr doc = args.targetDocument;
    const auto listFilepath = args.filepaths;
    TaskProgress* rootProgress = args.progress ? args.progress : &TaskProgress::null();
    Messenger* messenger = args.messenger ? args.messenger : &Messenger::null();

    bool ok = true;

    using ReaderPtr = std::unique_ptr<Reader>;
    struct TaskData {
        ReaderPtr reader;
        FilePath filepath;
        Format fileFormat = Format_Unknown;
        TaskProgress* progress = nullptr;
        TaskId taskId = 0;
        TDF_LabelSequence seqTransferredEntity;
        bool readSuccess = false;
        bool transferred = false;
        MessageCollecter messenger;
    };

    auto fnEntityPostProcessRequired = [&](Format format) {
        if (args.entityPostProcess && args.entityPostProcessRequiredIf)
            return args.entityPostProcessRequiredIf(format);
        else
            return false;
    };
    auto fnAddError = [&](TaskData& taskData, std::string_view errorMsg) {
        ok = false;
        taskData.messenger.error() << fmt::format(
            textIdTr("Error during import of '{}'\n{}"), taskData.filepath.u8string(), errorMsg
        );
    };
    auto fnReadFileError = [&](TaskData& taskData, std::string_view errorMsg) {
        fnAddError(taskData, errorMsg);
        return false;
    };
    auto fnReadFile = [&](TaskData& taskData) {
        taskData.fileFormat = this->probeFormat(taskData.filepath);
        if (taskData.fileFormat == Format_Unknown)
            return fnReadFileError(taskData, textIdTr("Unknown format"));

        double portionSize = 40;
        if (fnEntityPostProcessRequired(taskData.fileFormat))
            portionSize *= (100 - args.entityPostProcessProgressSize) / 100.;

        TaskProgress progress(taskData.progress, portionSize, textIdTr("Reading file"));
        taskData.reader = this->createReader(taskData.fileFormat);
        if (!taskData.reader)
            return fnReadFileError(taskData, textIdTr("No supporting reader"));

        taskData.reader->setMessenger(&taskData.messenger);
        if (args.parametersProvider) {
            taskData.reader->applyProperties(
                args.parametersProvider->findReaderParameters(taskData.fileFormat)
            );
        }

        if (!taskData.reader->readFile(taskData.filepath, &progress))
            return fnReadFileError(taskData, textIdTr("File read problem"));

        return true;
    };
    auto fnTransfer = [&](TaskData& taskData) {
        double portionSize = 60;
        if (fnEntityPostProcessRequired(taskData.fileFormat))
            portionSize *= (100 - args.entityPostProcessProgressSize) / 100.;

        TaskProgress progress(taskData.progress, portionSize, textIdTr("Transferring file"));
        if (taskData.reader && !TaskProgress::isAbortRequested(&progress)) {
            taskData.seqTransferredEntity = taskData.reader->transfer(doc, &progress);
            if (taskData.seqTransferredEntity.IsEmpty())
                fnAddError(taskData, textIdTr("File transfer problem"));
        }

        taskData.transferred = true;
    };
    auto fnPostProcess = [&](TaskData& taskData) {
        if (!fnEntityPostProcessRequired(taskData.fileFormat))
            return;

        TaskProgress progress(
            taskData.progress, args.entityPostProcessProgressSize, args.entityPostProcessProgressStep
        );
        const double subPortionSize = 100. / double(taskData.seqTransferredEntity.Size());
        for (const TDF_Label& labelEntity : taskData.seqTransferredEntity) {
            TaskProgress subProgress(&progress, subPortionSize);
            args.entityPostProcess(labelEntity, &subProgress);
        }
    };
    auto fnAddModelTreeEntities = [&](const TaskData& taskData) {
        // Need to call Document::addEntityTreeNodeSequence() instead of addEntityTreeNode() in
        // for() loop. The former function doesn't interleave update of the model tree and emission
        // of "entity added" signal for each entity. This prevents data race to happen on the
        // Document's model tree within slots connected to signal(and living in other threads)
        doc->addEntityTreeNodeSequence(taskData.seqTransferredEntity);
    };
    auto fnDispatchMessages = [=](TaskData& taskData) {
        const auto strFilepath = taskData.filepath.make_preferred().u8string();
        dispatchWarnings(
            fmt::format("Warning(s) during import from '{}'", strFilepath),
            taskData.messenger, messenger
        );
        dispatchErrors(
            fmt::format("Errors(s) during import from '{}'", strFilepath),
            taskData.messenger, messenger
        );
        taskData.messenger.clear();
    };

    if (listFilepath.size() == 1) { // Single file case
        TaskData taskData;
        taskData.filepath = listFilepath.front();
        taskData.progress = rootProgress;
        ok = fnReadFile(taskData);
        if (ok) {
            fnTransfer(taskData);
            fnPostProcess(taskData);
            fnAddModelTreeEntities(taskData);
        }

        fnDispatchMessages(taskData);
    }
    else { // Many files case
        std::vector<TaskData> vecTaskData;
        vecTaskData.resize(listFilepath.size());

        TaskManager childTaskManager;
        childTaskManager.signalProgressChanged.connectSlot([&](TaskId, int) {
            rootProgress->setValue(childTaskManager.globalProgress());
        });

        // Read files
        for (TaskData& taskData : vecTaskData) {
            taskData.filepath = listFilepath[&taskData - &vecTaskData.front()];
            taskData.taskId = childTaskManager.newTask([&](TaskProgress* progressChild) {
                taskData.progress = progressChild;
                taskData.readSuccess = fnReadFile(taskData);
            });
        }

        for (const TaskData& taskData : vecTaskData)
            childTaskManager.run(taskData.taskId, TaskAutoDestroy::Off);

        // Transfer to document
        auto taskDataCount = CppUtils::safeStaticCast<int>(vecTaskData.size());
        while (taskDataCount > 0 && !rootProgress->isAbortRequested()) {
            auto it = std::find_if(vecTaskData.begin(), vecTaskData.end(), [&](const TaskData& taskData) {
                return !taskData.transferred && childTaskManager.waitForDone(taskData.taskId, 25);
            });

            if (it != vecTaskData.end()) {
                if (it->readSuccess) {
                    fnTransfer(*it);
                    fnPostProcess(*it);
                    fnAddModelTreeEntities(*it);
                }

                fnDispatchMessages(*it);
                --taskDataCount;
            }
        } // endwhile
    }

    return ok;
}

System::Operation_ImportInDocument System::importInDocument() const
{
    return Operation_ImportInDocument(*this);
}

bool System::exportApplicationItems(const Args_ExportApplicationItems& args) const
{
    TaskProgress* progress = args.progress ? args.progress : &TaskProgress::null();
    MessageCollecter msgCollect;
    auto fnError = [&](std::string_view errorMsg) {
        msgCollect.error() << errorMsg;
        return false;
    };

    auto _ = gsl::finally([&]{
        Messenger* messenger = args.messenger ? args.messenger : &Messenger::null();
        const std::string strFilepath = args.targetFilepath.u8string();
        dispatchWarnings(fmt::format("Warning(s) during export to '{}'", strFilepath), msgCollect, messenger);
        dispatchErrors(fmt::format("Errors(s) during export to '{}'", strFilepath), msgCollect, messenger);
    });

    std::unique_ptr<Writer> writer = this->createWriter(args.targetFormat);
    if (!writer)
        return fnError(textIdTr("No supporting writer"));

    writer->setMessenger(&msgCollect);
    writer->applyProperties(args.parameters);
    {
        TaskProgress transferProgress(progress, 40, textIdTr("Transfer"));
        const bool okTransfer = writer->transfer(args.applicationItems, &transferProgress);
        if (!okTransfer)
            return fnError(textIdTr("File transfer problem"));
    }

    {
        TaskProgress writeProgress(progress, 60, textIdTr("Write"));
        const bool okWriteFile = writer->writeFile(args.targetFilepath, &writeProgress);
        if (!okWriteFile)
            return fnError(textIdTr("File write problem"));
    }

    return true;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::targetFile(const FilePath& filepath)
{
    m_args.targetFilepath = filepath;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::targetFormat(Format format)
{
    m_args.targetFormat = format;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withItem(const ApplicationItem& appItem)
{
    m_args.applicationItems = { &appItem, 1 };
    return *this;
}


System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withItems(Span<const ApplicationItem> appItems)
{
    m_args.applicationItems = appItems;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withParameters(const PropertyGroup* parameters)
{
    m_args.parameters = parameters;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withMessenger(Messenger* messenger)
{
    m_args.messenger = messenger;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withTaskProgress(TaskProgress* progress)
{
    m_args.progress = progress;
    return *this;
}

bool System::Operation_ExportApplicationItems::execute()
{
    return m_system.exportApplicationItems(m_args);
}

System::Operation_ExportApplicationItems::Operation_ExportApplicationItems(const System& system)
    : m_system(system)
{
}

System::Operation_ExportApplicationItems System::exportApplicationItems() const
{
    return Operation_ExportApplicationItems(*this);
}

void System::visitUniqueItems(
        Span<const ApplicationItem> spanItem,
        std::function<void (const ApplicationItem&)> fnCallback
    )
{
    std::unordered_set<DocumentPtr> setDoc;
    for (const ApplicationItem& item : spanItem) {
        if (item.isDocument() && item.document()->entityCount() > 0) {
            auto [it, ok] = setDoc.insert(item.document());
            if (ok)
                fnCallback(item);
        }
    }

    std::unordered_set<TDF_Label> setNode;
    for (const ApplicationItem& item : spanItem) {
        if (item.isDocumentTreeNode()) {
            auto itDoc = setDoc.find(item.document());
            if (itDoc == setDoc.cend()) {
                auto [it, ok] = setNode.insert(item.documentTreeNode().label());
                if (ok)
                    fnCallback(item);
            }
        }
    }
}

void System::traverseUniqueItems(
        Span<const ApplicationItem> spanItem,
        std::function<void(const DocumentTreeNode&)> fnCallback,
        TreeTraversal mode
    )
{
    System::visitUniqueItems(spanItem, [=](const ApplicationItem& item) {
        const DocumentPtr doc = item.document();
        const Tree<TDF_Label>& modelTree = doc->modelTree();
        if (item.isDocument()) {
            traverseTree(modelTree, [&](TreeNodeId id) { fnCallback({ doc, id }); }, mode);
        }
        else if (item.isDocumentTreeNode()) {
            const TreeNodeId docTreeNodeId = item.documentTreeNode().id();
            traverseTree(docTreeNodeId, modelTree, [&](TreeNodeId id) { fnCallback({ doc, id }); }, mode);
        }
    });
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::targetDocument(const DocumentPtr& document)
{
    m_args.targetDocument = document;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withFilepaths(Span<const FilePath> filepaths)
{
    m_args.filepaths = filepaths;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withParametersProvider(const ParametersProvider* provider)
{
    m_args.parametersProvider = provider;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withMessenger(Messenger* messenger)
{
    m_args.messenger = messenger;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withTaskProgress(TaskProgress* progress)
{
    m_args.progress = progress;
    return *this;
}

System::Operation_ImportInDocument::Operation&
System::Operation_ImportInDocument::withFilepath(const FilePath& filepath)
{
    return this->withFilepaths(Span<const FilePath>(&filepath, 1));
}

System::Operation_ImportInDocument::Operation&
System::Operation_ImportInDocument::withEntityPostProcess(std::function<void (TDF_Label, TaskProgress*)> fn)
{
    m_args.entityPostProcess = std::move(fn);
    return *this;
}

System::Operation_ImportInDocument::Operation&
System::Operation_ImportInDocument::withEntityPostProcessRequiredIf(std::function<bool(Format)> fn)
{
    m_args.entityPostProcessRequiredIf = std::move(fn);
    return *this;
}

System::Operation_ImportInDocument::Operation&
System::Operation_ImportInDocument::withEntityPostProcessInfoProgress(int progressSize, std::string_view progressStep)
{
    m_args.entityPostProcessProgressSize = progressSize;
    m_args.entityPostProcessProgressStep = progressStep;
    return *this;
}

bool System::Operation_ImportInDocument::execute()
{
    return m_system.importInDocument(m_args);
}

System::Operation_ImportInDocument::Operation_ImportInDocument(const System& system)
    : m_system(system)
{
}

namespace {

bool matchRegExp_atStart(std::string_view str, const std::regex& rx)
{
    std::match_results<std::string_view::const_iterator> mres;
    const bool match = std::regex_search(str.cbegin(), str.cend(), mres, rx);
    return match ? mres.position() == 0 : false;
}

bool matchRegExp_anyWhere(std::string_view str, const std::regex& rx)
{
    return std::regex_search(str.cbegin(), str.cend(), rx);
}

} // namespace

Format probeFormat_STEP(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"(^\s*ISO-10303-21\s*;\s*HEADER)" };
    return matchRegExp_atStart(input.contentsBegin, rx) ? Format_STEP : Format_Unknown;
}

Format probeFormat_IGES(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"(^.{72}S\s*[0-9]+\s*[\n\r\f])" };
    return matchRegExp_atStart(input.contentsBegin, rx) ? Format_IGES : Format_Unknown;
}

Format probeFormat_OCCBREP(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"(^\s*DBRep_DrawableShape)" };
    return matchRegExp_atStart(input.contentsBegin, rx) ? Format_OCCBREP : Format_Unknown;
}

Format probeFormat_STL(const System::FormatProbeInput& input)
{
    std::string_view sample = input.contentsBegin;
    // Binary STL ?
    {
        constexpr size_t binaryStlHeaderSize = 80 + sizeof(uint32_t);
        if (sample.size() >= binaryStlHeaderSize) {
            constexpr uint32_t offset = 80; // Skip header
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(sample.data());
            const uint32_t facetsCount =
                    bytes[offset]
                    | (bytes[offset+1] << 8)
                    | (bytes[offset+2] << 16)
                    | (bytes[offset+3] << 24)
                ;
            constexpr unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
            if ((facetSize * facetsCount + binaryStlHeaderSize) == input.hintFullSize)
                return Format_STL;
        }
    }

    // ASCII STL ?
    {
        const std::regex rx{ R"(^\s*solid\s+)" };
        if (matchRegExp_atStart(input.contentsBegin, rx))
            return Format_STL;
    }

    return Format_Unknown;
}

Format probeFormat_OBJ(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"([^\n]\s*(v|vt|vn|vp|surf)\s+[-\+]?[0-9\.]+\s)" };
    return matchRegExp_anyWhere(input.contentsBegin, rx) ? Format_OBJ : Format_Unknown;
}

Format probeFormat_PLY(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"(^\s*ply\s+format\s+(ascii|binary_little_endian|binary_big_endian)\s+)" };
    return matchRegExp_atStart(input.contentsBegin, rx) ? Format_PLY : Format_Unknown;
}

Format probeFormat_OFF(const System::FormatProbeInput& input)
{
    const std::regex rx{ R"(^\s*[CN4]?OFF\s+)" };
    return matchRegExp_atStart(input.contentsBegin, rx) ? Format_OFF : Format_Unknown;
}

void addPredefinedFormatProbes(System* system)
{
    if (!system)
        return;

    system->addFormatProbe(probeFormat_STEP);
    system->addFormatProbe(probeFormat_IGES);
    system->addFormatProbe(probeFormat_OCCBREP);
    system->addFormatProbe(probeFormat_STL);
    system->addFormatProbe(probeFormat_OBJ);
    system->addFormatProbe(probeFormat_PLY);
    system->addFormatProbe(probeFormat_OFF);
}

} // namespace Mayo::IO
