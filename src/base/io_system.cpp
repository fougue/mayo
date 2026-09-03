/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "io_system.h"

#include "caf_utils.h" // At least for std::hash<TDF_Label>
#include "document.h"
#include "io_parameters_provider.h"
#include "message_collecter.h"
#include "task_manager.h"
#include "task_progress.h"
#include "tkernel_utils.h"
#include "thread_messenger_channel.h"

#include <fmt/format.h>
#include <gsl/util>

#include <algorithm>
#include <fstream>
#include <locale>
#include <optional>
#include <regex>
#include <type_traits>
#include <unordered_set>
#include <variant>
#include <vector>

namespace Mayo::IO {

namespace {

bool containsFormat(gsl::span<const Format> spanFormat, Format format)
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

bool isEntityPostProcessRequired(Format format, const System::ArgsImport& args)
{
    if (args.entityPostProcess && args.entityPostProcessRequiredIf)
        return args.entityPostProcessRequiredIf(format);
    else
        return false;
}

// Executes a callable while safely handling exceptions
// Invokes the provided callable and returns its result wrapped in a `std::optional`
// Any exception thrown during execution is caught and reported through the provided `Messenger`
template<typename Function>
auto noThrowExec(Messenger* messenger, Function fn)
{
    using FnReturnType = std::invoke_result_t<Function>;
    using ReturnType = std::conditional_t<std::is_void_v<FnReturnType>, std::monostate, FnReturnType>;

    messenger = messenger ? messenger : &Messenger::null();

    try {
        if constexpr (std::is_void_v<FnReturnType>) {
            fn();
            return std::optional<ReturnType>{std::monostate{}};
        } else {
            return std::optional<ReturnType>{fn()};
        }
    }
    catch (const Standard_Failure& err) {
        messenger->emitError(fmt::format(
            System::textIdTr("Exception '{}' : {}"),
            TKernelUtils::errorTypeName(err), TKernelUtils::errorMessage(err)
            ));
    }
    catch (const std::exception& err) {
        messenger->emitError(fmt::format(System::textIdTr("Exception : {}"), err.what()));
    }
    catch (...) {
        messenger->emitError(System::textIdTr("Unknown exception"));
    }

    return std::optional<ReturnType>{};
}

struct ImportTaskData {
    std::unique_ptr<Reader> reader;
    FilePath filepath;
    Format fileFormat = Format_Unknown;
    TaskProgress* progress = nullptr;
    TaskId taskId = 0;
    NCollection_Sequence<TDF_Label> seqTransferredEntity;
    bool readSuccess = false;
    bool transferred = false; // Is transfer done ?
    MessageCollecter messenger;
};

bool success(const ImportTaskData& taskData)
{
    return taskData.readSuccess && !taskData.seqTransferredEntity.IsEmpty();
}

void readFile(ImportTaskData& taskData, const System& ioSystem, const System::ArgsImport& args)
{
    auto error = [&](std::string_view trErrorMsg) {
        taskData.messenger.error() << trErrorMsg;
        taskData.readSuccess = false;
    };

    taskData.fileFormat = ioSystem.probeFormat(taskData.filepath);
    if (taskData.fileFormat == Format_Unknown)
        return error(System::textIdTr("Unknown format"));

    double portionSize = 40;
    if (isEntityPostProcessRequired(taskData.fileFormat, args))
        portionSize *= (100 - args.entityPostProcessProgressSize) / 100.;

    TaskProgress progress(taskData.progress, portionSize, System::textIdTr("Reading file"));
    taskData.reader = ioSystem.createReader(taskData.fileFormat);
    if (!taskData.reader)
        return error(System::textIdTr("No supporting reader"));

    taskData.reader->setMessenger(&taskData.messenger);
    if (args.parametersProvider) {
        taskData.reader->applyProperties(
            args.parametersProvider->findReaderParameters(taskData.fileFormat)
        );
    }

    // Enable forwarding of global OCCT messages (eg Message::SendFail()) to the Mayo messenger
    [[maybe_unused]] ThreadMessengerChannel::Scope scopeMsg(&taskData.messenger);

    auto readFile = noThrowExec(&taskData.messenger, [&]{
        return taskData.reader->readFile(taskData.filepath, &progress);
    });
    if (!readFile.value_or(false))
        return error(System::textIdTr("File read problem"));

    taskData.readSuccess = true;
}

void transfer(ImportTaskData& taskData, const System::ArgsImport& args)
{
    if (!taskData.readSuccess)
        return;

    double portionSize = 60;
    if (isEntityPostProcessRequired(taskData.fileFormat, args))
        portionSize *= (100 - args.entityPostProcessProgressSize) / 100.;

    TaskProgress progress(taskData.progress, portionSize, System::textIdTr("Transferring file"));
    if (taskData.reader && !TaskProgress::isAbortRequested(&progress)) {
        // Enable forwarding of global OCCT messages (eg Message::SendFail()) to the Mayo messenger
        [[maybe_unused]] ThreadMessengerChannel::Scope scopeMsg(&taskData.messenger);

        auto transfer = noThrowExec(&taskData.messenger, [&]{
            return taskData.reader->transfer(args.targetDocument, &progress);
        });
        taskData.seqTransferredEntity = transfer.value_or(NCollection_Sequence<TDF_Label>{});
        if (taskData.seqTransferredEntity.IsEmpty())
            taskData.messenger.error() << System::textIdTr("File transfer problem, no entity imported");
    }

    taskData.transferred = true;
}

void postProcess(ImportTaskData& taskData, const System::ArgsImport& args)
{
    if (!success(taskData))
        return;

    if (!isEntityPostProcessRequired(taskData.fileFormat, args))
        return;

    TaskProgress progress(
        taskData.progress, args.entityPostProcessProgressSize, args.entityPostProcessProgressStep
    );
    const double subPortionSize = 100. / static_cast<double>(taskData.seqTransferredEntity.Size());
    for (const TDF_Label& labelEntity : taskData.seqTransferredEntity) {
        TaskProgress subProgress(&progress, subPortionSize);
        noThrowExec(&taskData.messenger, [&]{ args.entityPostProcess(labelEntity, &subProgress); });
    }
}

void addModelTreeEntities(const ImportTaskData& taskData, const DocumentPtr& targetDoc)
{
    if (!success(taskData))
        return;

    // Need to call Document::addEntityTreeNodeSequence() instead of addEntityTreeNode() in
    // for() loop. The former function doesn't interleave update of the model tree and emission
    // of "entity added" signal for each entity. This prevents data race to happen on the
    // Document's model tree within slots connected to signal(and living in other threads)
    targetDoc->addEntityTreeNodeSequence(taskData.seqTransferredEntity);
}

void dispatchMessages(ImportTaskData& taskData, Messenger* targetMessenger)
{
    const auto strFilepath = taskData.filepath.make_preferred().u8string();
    dispatchWarnings(
        fmt::format("Warning(s) during import of '{}'", strFilepath),
        taskData.messenger, targetMessenger
    );
    dispatchErrors(
        fmt::format("Errors(s) during import of '{}'", strFilepath),
        taskData.messenger, targetMessenger
    );
    taskData.messenger.clear();
}

} // namespace

System::System()
{
    ThreadMessengerChannel::addGlobalOccPrinter();
}

void System::addFormatProbe(const FormatProbe& probe)
{
    m_vecFormatProbe.push_back(probe);
}

System::FormatProbeInput System::getFormatProbeInput(const FilePath& filepath, gsl::span<char> buff)
{
    FormatProbeInput probeInput = {};
    probeInput.filepath = filepath;

    std::ifstream file;
    file.open(filepath);
    if (!file.is_open())
        return probeInput;

    file.read(buff.data(), buff.size());
    probeInput.contentsBegin = std::string_view(buff.data(), file.gcount());
    probeInput.hintFullSize = filepathFileSize(filepath);
    return probeInput;
}

Format System::probeFormat(const FilePath& filepath) const
{
    // Try to guess from file contents
    char buff[2048] = {};
    auto probeInput = getFormatProbeInput(filepath, buff);
    if (!probeInput.contentsBegin.empty()) {
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
    auto fnMatchFileSuffix = [&](Format format) {
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

bool System::importInDocument(const ArgsImport& args) const
{
    TaskProgress* rootProgress = args.progress ? args.progress : &TaskProgress::null();
    Messenger* messenger = args.messenger ? args.messenger : &Messenger::null();

    if (args.filepaths.size() == 1) {
        // Single file case
        ImportTaskData taskData;
        taskData.filepath = args.filepaths.front();
        taskData.progress = rootProgress;
        readFile(taskData, *this, args);
        transfer(taskData, args);
        postProcess(taskData, args);
        addModelTreeEntities(taskData, args.targetDocument);
        dispatchMessages(taskData, messenger);
        return success(taskData);
    }
    else {
        // Many files case
        std::vector<ImportTaskData> vecTaskData;
        vecTaskData.resize(args.filepaths.size());

        TaskManager childTaskManager;
        childTaskManager.signalProgressChanged.connectSlot([&](TaskId, double) {
            rootProgress->setValue(childTaskManager.globalProgress());
        });

        // Read files
        for (ImportTaskData& taskData : vecTaskData) {
            taskData.filepath = args.filepaths[&taskData - &vecTaskData.front()];
            taskData.taskId = childTaskManager.newTask([&](TaskProgress* progressChild) {
                taskData.progress = progressChild;
                readFile(taskData, *this, args);
            });
        }

        for (const ImportTaskData& taskData : vecTaskData)
            childTaskManager.run(taskData.taskId, TaskAutoDestroy::Off);

        // Transfer to target document
        auto taskDataCount = static_cast<int>(vecTaskData.size());
        while (taskDataCount > 0 && !rootProgress->isAbortRequested()) {
            auto it = std::find_if(vecTaskData.begin(), vecTaskData.end(), [&](const ImportTaskData& taskData) {
                return !taskData.transferred && childTaskManager.waitForDone(taskData.taskId, 25);
            });
            if (it != vecTaskData.end()) {
                transfer(*it, args);
                postProcess(*it, args);
                addModelTreeEntities(*it, args.targetDocument);
                dispatchMessages(*it, messenger);
                --taskDataCount;
            }
        } // endwhile

        for (const ImportTaskData& taskData : vecTaskData) {
            if (!success(taskData))
                return false;
        }

        return true;
    }
}

bool System::importInDocument(const DocumentPtr& targetDoc, const FilePath& file)
{
    return this->importInDocument(ArgsImport().setTargetDocument(targetDoc).setFilepath(file));
}

bool System::exportItems(const ArgsExport& args) const
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

    // Enable forwarding of global OCCT messages (eg Message::SendFail()) to the Mayo messenger
    [[maybe_unused]] ThreadMessengerChannel::Scope scopeMsg(&msgCollect);

    {
        TaskProgress transferProgress(progress, 40, textIdTr("Transfer"));
        auto transfer = noThrowExec(&msgCollect, [&]{
            return writer->transfer(args.applicationItems, &transferProgress);
        });
        if (!transfer.value_or(false))
            return fnError(textIdTr("File transfer problem"));
    }

    {
        TaskProgress writeProgress(progress, 60, textIdTr("Write"));
        auto writeFile = noThrowExec(&msgCollect, [&]{
            return writer->writeFile(args.targetFilepath, &writeProgress);
        });
        if (!writeFile.value_or(false))
            return fnError(textIdTr("File write problem"));
    }

    return true;
}

System::ArgsExport& System::ArgsExport::setTargetFile(const FilePath& filepath)
{
    this->targetFilepath = filepath;
    return *this;
}

System::ArgsExport& System::ArgsExport::setTargetFormat(Format format)
{
    this->targetFormat = format;
    return *this;
}

System::ArgsExport& System::ArgsExport::setItem(const ApplicationItem& appItem)
{
    this->applicationItems = { &appItem, 1 };
    return *this;
}


System::ArgsExport& System::ArgsExport::setItems(gsl::span<const ApplicationItem> appItems)
{
    this->applicationItems = appItems;
    return *this;
}

System::ArgsExport& System::ArgsExport::setParameters(const PropertyGroup* parameters)
{
    this->parameters = parameters;
    return *this;
}

System::ArgsExport& System::ArgsExport::setMessenger(Messenger* messenger)
{
    this->messenger = messenger;
    return *this;
}

System::ArgsExport& System::ArgsExport::setTaskProgress(TaskProgress* progress)
{
    this->progress = progress;
    return *this;
}

void System::visitUniqueItems(
        gsl::span<const ApplicationItem> spanItem,
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
        gsl::span<const ApplicationItem> spanItem,
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

System::ArgsImport& System::ArgsImport::setTargetDocument(const DocumentPtr& document)
{
    this->targetDocument = document;
    return *this;
}

System::ArgsImport& System::ArgsImport::setFilepath(const FilePath& filepath)
{
    return this->setFilepaths(gsl::span<const FilePath>(&filepath, 1));
}

System::ArgsImport& System::ArgsImport::setFilepaths(gsl::span<const FilePath> filepaths)
{
    this->filepaths = filepaths;
    return *this;
}

System::ArgsImport& System::ArgsImport::setParametersProvider(const ParametersProvider* provider)
{
    this->parametersProvider = provider;
    return *this;
}

System::ArgsImport& System::ArgsImport::setMessenger(Messenger* messenger)
{
    this->messenger = messenger;
    return *this;
}

System::ArgsImport& System::ArgsImport::setTaskProgress(TaskProgress* progress)
{
    this->progress = progress;
    return *this;
}

System::ArgsImport& System::ArgsImport::setEntityPostProcess(std::function<void(TDF_Label, TaskProgress*)> fn)
{
    this->entityPostProcess = std::move(fn);
    return *this;
}

System::ArgsImport& System::ArgsImport::setEntityPostProcessRequiredIf(std::function<bool(Format)> fn)
{
    this->entityPostProcessRequiredIf = std::move(fn);
    return *this;
}

System::ArgsImport& System::ArgsImport::setEntityPostProcessInfoProgress(int progressSize, std::string_view progressStep)
{
    this->entityPostProcessProgressSize = progressSize;
    this->entityPostProcessProgressStep = progressStep;
    return *this;
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
    const bool isFormatOCCBREP = isFormatAscii_OCCBREP(input) || isFormatBinary_OCCBREP(input);
    return isFormatOCCBREP ? Format_OCCBREP : Format_Unknown;
}

Format probeFormat_OCCXCAF(const System::FormatProbeInput& input)
{
    // Binary XCAF starts with "BINFILE" which is too short for a reliable identification...
    const std::regex rxXml{ R"(^\s*"<document format=\"XmlXCAF\"")" };
    return matchRegExp_atStart(input.contentsBegin, rxXml) ? Format_OCCXCAF : Format_Unknown;
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
    system->addFormatProbe(probeFormat_OCCXCAF);
    system->addFormatProbe(probeFormat_STL);
    system->addFormatProbe(probeFormat_OBJ);
    system->addFormatProbe(probeFormat_PLY);
    system->addFormatProbe(probeFormat_OFF);
}

bool isFormatAscii_OCCBREP(const System::FormatProbeInput& input)
{
    const std::regex rxAscii{ R"(^\s*DBRep_DrawableShape)" };
    return matchRegExp_atStart(input.contentsBegin, rxAscii);
}

bool isFormatBinary_OCCBREP(const System::FormatProbeInput& input)
{
    const std::regex rxBin{ R"(^\s*Open CASCADE Topology V[0-9])" };
    return matchRegExp_atStart(input.contentsBegin, rxBin);
}

} // namespace Mayo::IO
