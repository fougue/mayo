/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_system.h"

#include "document.h"
#include "io_parameters_provider.h"
#include "io_reader.h"
#include "io_writer.h"
#include "messenger.h"
#include "task_manager.h"
#include "task_progress.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <algorithm>
#include <array>
#include <future>
#include <locale>
#include <mutex>
#include <regex>

namespace Mayo {
namespace IO {

namespace {

TaskProgress* nullTaskProgress()
{
    static TaskProgress null;
    return &null;
}

Messenger* nullMessenger()
{
    return NullMessenger::instance();
}

bool containsFormat(Span<const Format> spanFormat, const Format& format)
{
    auto itFormat = std::find(spanFormat.cbegin(), spanFormat.cend(), format);
    return itFormat != spanFormat.cend();
}

} // namespace

void System::addFormatProbe(const FormatProbe& probe)
{
    m_vecFormatProbe.push_back(probe);
}

Format System::probeFormat(const QString& filepath) const
{
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
        std::array<char, 2048> buff;
        buff.fill(0);
        file.read(buff.data(), buff.size());
        FormatProbeInput probeInput = {};
        probeInput.filepath = filepath;
        probeInput.contentsBegin = QByteArray::fromRawData(buff.data(), buff.size());
        probeInput.hintFullSize = file.size();
        for (const FormatProbe& fnProbe : m_vecFormatProbe) {
            const Format format = fnProbe(probeInput);
            if (format != Format_Unknown)
                return format;
        }

        // Try to guess from file suffix
        const QString fileSuffix = QFileInfo(file).suffix();
        auto fnMatchFileSuffix = [=](const Format& format) {
            return format.fileSuffixes.contains(fileSuffix, Qt::CaseInsensitive);
        };
        for (const Format& format : m_vecReaderFormat) {
            if (fnMatchFileSuffix(format))
                return format;
        }

        for (const Format& format : m_vecWriterFormat) {
            if (fnMatchFileSuffix(format))
                return format;
        }
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

    for (const Format& format : ptr->formats()) {
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

    for (const IO::Format& format : ptr->formats()) {
        auto itFormat = std::find(m_vecWriterFormat.cbegin(), m_vecWriterFormat.cend(), format);
        if (itFormat == m_vecWriterFormat.cend())
            m_vecWriterFormat.push_back(format);
    }

    m_vecFactoryWriter.push_back(std::move(ptr));
}

const FactoryReader* System::findFactoryReader(const Format& format) const
{
    for (const std::unique_ptr<FactoryReader>& ptr : m_vecFactoryReader) {
        if (containsFormat(ptr->formats(), format))
            return ptr.get();
    }

    return nullptr;
}

const FactoryWriter* System::findFactoryWriter(const Format& format) const
{
    for (const std::unique_ptr<FactoryWriter>& ptr : m_vecFactoryWriter) {
        if (containsFormat(ptr->formats(), format))
            return ptr.get();
    }

    return nullptr;
}

std::unique_ptr<Reader> System::createReader(const Format& format) const
{
    const FactoryReader* ptr = this->findFactoryReader(format);
    if (ptr)
        return ptr->create(format);

    return {};
}

std::unique_ptr<Writer> System::createWriter(const Format& format) const
{
    const FactoryWriter* ptr = this->findFactoryWriter(format);
    if (ptr)
        return ptr->create(format);

    return {};
}

QString System::fileFilter(const Format& format)
{
    if (format == Format_Unknown)
        return QString();

    QString filter;
    for (const QString& suffix : format.fileSuffixes) {
        if (&suffix != &format.fileSuffixes.front())
            filter += " ";

        filter += "*." + suffix;
#ifdef Q_OS_UNIX
        filter += " *." + suffix.toUpper();
#endif
    }

    //: %1 is the format identifier and %2 is the file filters string
    return tr("%1 files(%2)")
            .arg(QString::fromLatin1(format.identifier))
            .arg(filter);
}

bool System::importInDocument(const Args_ImportInDocument& args)
{
    DocumentPtr doc = args.targetDocument;
    const QStringList listFilepath = args.filepaths;
    TaskProgress* progress = args.progress ? args.progress : nullTaskProgress();
    Messenger* messenger = args.messenger ? args.messenger : nullMessenger();

    bool ok = true;

    using ReaderPtr = std::unique_ptr<Reader>;
    auto fnAddError = [&](QString filepath, QString errorMsg) {
        ok = false;
        messenger->emitError(tr("Error during import of '%1'\n%2").arg(filepath, errorMsg));
    };
    auto fnReadFileError = [&](QString filepath, QString errorMsg) -> ReaderPtr {
        fnAddError(filepath, errorMsg);
        return {};
    };
    auto fnReadFile = [&](QString filepath, TaskProgress* subProgress) -> ReaderPtr {
        subProgress->beginScope(40, tr("Reading file"));
        auto _ = gsl::finally([=]{ subProgress->endScope(); });
        const Format fileFormat = this->probeFormat(filepath);
        if (fileFormat == Format_Unknown)
            return fnReadFileError(filepath, tr("Unknown format"));

        std::unique_ptr<Reader> reader = this->createReader(fileFormat);
        if (!reader)
            return fnReadFileError(filepath, tr("No supporting reader"));

        if (args.parametersProvider)
            reader->applyProperties(args.parametersProvider->findReaderParameters(fileFormat));

        if (!reader->readFile(filepath, subProgress))
            return fnReadFileError(filepath, tr("File read problem"));

        return reader;
    };
    auto fnTransfer = [&](QString filepath, const ReaderPtr& reader, TaskProgress* subProgress) {
        subProgress->beginScope(60, tr("Transferring file"));
        if (reader) {
            if (!reader->transfer(doc, subProgress) && !TaskProgress::isAbortRequested(subProgress))
                fnAddError(filepath, tr("File transfer problem"));
        }

        subProgress->endScope();
    };

    if (listFilepath.size() == 1) { // Single file case
        const ReaderPtr reader = fnReadFile(listFilepath.front(), progress);
        fnTransfer(listFilepath.front(), reader, progress);
    }
    else { // Many files case
        struct TaskData {
            std::unique_ptr<Reader> reader;
            QString filepath;
            TaskProgress* progress = nullptr;
            TaskId taskId = 0;
            bool transferred = false;
        };
        std::vector<TaskData> vecTaskData;
        vecTaskData.resize(listFilepath.size());

        TaskManager childTaskManager;
        QObject::connect(
                    &childTaskManager, &TaskManager::progressChanged,
                    [&](TaskId, int) { progress->setValue(childTaskManager.globalProgress()); });

        for (int i = 0; i < listFilepath.size(); ++i) {
            TaskData& taskData = vecTaskData.at(i);
            taskData.filepath = listFilepath.at(i);
            const TaskId childTaskId = childTaskManager.newTask([&](TaskProgress* progressChild) {
                taskData.progress = progressChild;
                taskData.reader = fnReadFile(taskData.filepath, progressChild);
            });
            taskData.taskId = childTaskId;
            childTaskManager.run(childTaskId, TaskAutoDestroy::Off);
        }

        // Transfer to document
        int taskDataCount = vecTaskData.size();
        while (taskDataCount > 0 && (!progress || !progress->isAbortRequested())) {
            auto itTaskData = std::find_if(
                        vecTaskData.begin(), vecTaskData.end(),
                        [&](const TaskData& taskData) {
                return !taskData.transferred && childTaskManager.waitForDone(taskData.taskId, 10);
            });
            if (itTaskData == vecTaskData.end()) {
                itTaskData = std::find_if(
                            vecTaskData.begin(), vecTaskData.end(),
                            [&](const TaskData& taskData) { return !taskData.transferred; });
                if (itTaskData != vecTaskData.end())
                    if (!childTaskManager.waitForDone(itTaskData->taskId, 100))
                        itTaskData = vecTaskData.end();
            }

            if (itTaskData != vecTaskData.end()) {
                fnTransfer(itTaskData->filepath, itTaskData->reader, itTaskData->progress);
                itTaskData->transferred = true;
                --taskDataCount;
            }
        } // endwhile
    }

    return ok;
}

System::Operation_ImportInDocument System::importInDocument() {
    return Operation_ImportInDocument(*this);
}

bool System::exportApplicationItems(const Args_ExportApplicationItems& args)
{
    TaskProgress* progress = args.progress ? args.progress : nullTaskProgress();
    Messenger* messenger = args.messenger ? args.messenger : nullMessenger();
    auto fnError = [=](const QString& errorMsg) {
        messenger->emitError(tr("Error during export to '%1'\n%2").arg(args.targetFilepath, errorMsg));
        return false;
    };

    std::unique_ptr<Writer> writer = this->createWriter(args.targetFormat);
    if (!writer)
        return fnError(tr("No supporting writer"));

    writer->applyProperties(args.parameters);
    auto _ = gsl::finally([=]{ progress->endScope(); });
    progress->beginScope(40, tr("Transfer"));
    const bool okTransfer = writer->transfer(args.applicationItems, progress);
    if (!okTransfer)
        return fnError(tr("File transfer problem"));

    progress->endScope();
    progress->beginScope(60, tr("Write"));
    const bool okWriteFile = writer->writeFile(args.targetFilepath, progress);
    if (!okWriteFile)
        return fnError(tr("File write problem"));

    return true;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::targetFile(const QString& filepath) {
    m_args.targetFilepath = filepath;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::targetFormat(const Format& format) {
    m_args.targetFormat = format;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withItems(Span<const ApplicationItem> appItems) {
    m_args.applicationItems = appItems;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withParameters(const PropertyGroup* parameters) {
    m_args.parameters = parameters;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withMessenger(Messenger* messenger) {
    m_args.messenger = messenger;
    return *this;
}

System::Operation_ExportApplicationItems&
System::Operation_ExportApplicationItems::withTaskProgress(TaskProgress* progress) {
    m_args.progress = progress;
    return *this;
}

bool System::Operation_ExportApplicationItems::execute() {
    return m_system.exportApplicationItems(m_args);
}

System::Operation_ExportApplicationItems::Operation_ExportApplicationItems(System& system)
    : m_system(system)
{
}

System::Operation_ExportApplicationItems System::exportApplicationItems()
{
    return Operation_ExportApplicationItems(*this);
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::targetDocument(const DocumentPtr& document) {
    m_args.targetDocument = document;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withFilepaths(const QStringList& filepaths) {
    m_args.filepaths = filepaths;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withParametersProvider(const ParametersProvider* provider) {
    m_args.parametersProvider = provider;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withMessenger(Messenger* messenger) {
    m_args.messenger = messenger;
    return *this;
}

System::Operation_ImportInDocument&
System::Operation_ImportInDocument::withTaskProgress(TaskProgress* progress) {
    m_args.progress = progress;
    return *this;
}

System::Operation_ImportInDocument::Operation&
System::Operation_ImportInDocument::withFilepath(const QString& filepath)
{
    return this->withFilepaths({ filepath });
}

bool System::Operation_ImportInDocument::execute() {
    return m_system.importInDocument(m_args);
}

System::Operation_ImportInDocument::Operation_ImportInDocument(System& system)
    : m_system(system)
{
}

namespace {

bool isSpace(char c) {
    return std::isspace(c, std::locale::classic());
}

bool matchToken(QByteArray::const_iterator itBegin, std::string_view token) {
    return std::strncmp(&(*itBegin), token.data(), token.size()) == 0;
}

auto findFirstNonSpace(const QByteArray& str) {
    return std::find_if_not(str.cbegin(), str.cend(), isSpace);
}

} // namespace

Format probeFormat_STEP(const System::FormatProbeInput& input)
{
    const QByteArray& sample = input.contentsBegin;
    // regex : ^\s*ISO-10303-21\s*;\s*HEADER
    constexpr std::string_view stepIsoId = "ISO-10303-21";
    constexpr std::string_view stepHeaderToken = "HEADER";
    auto itContentsBegin = findFirstNonSpace(sample);
    if (matchToken(itContentsBegin, stepIsoId)) {
        auto itChar = std::find_if_not(itContentsBegin + stepIsoId.size(), sample.cend(), isSpace);
        if (itChar != sample.cend() && *itChar == ';') {
            itChar = std::find_if_not(itChar + 1, sample.cend(), isSpace);
            if (matchToken(itChar, stepHeaderToken))
                return Format_STEP;
        }
    }

    return Format_Unknown;
}

Format probeFormat_IGES(const System::FormatProbeInput& input)
{
    const QByteArray& sample = input.contentsBegin;
    // regex : ^.{72}S\s*[0-9]+\s*[\n\r\f]
    bool isIges = true;
    if (sample.size() >= 80 && sample[72] == 'S') {
        for (int i = 73; i < 80 && isIges; ++i) {
            if (sample[i] != ' ' && !std::isdigit(static_cast<unsigned char>(sample[i])))
                isIges = false;
        }

        const char c80 = sample[80];
        if (isIges && (c80 == '\n' || c80 == '\r' || c80 == '\f')) {
            const int sVal = std::atoi(sample.data() + 73);
            if (sVal == 1)
                return Format_IGES;
        }
    }

    return Format_Unknown;
}

Format probeFormat_OCCBREP(const System::FormatProbeInput& input)
{
    // regex : ^\s*DBRep_DrawableShape
    auto itContentsBegin = findFirstNonSpace(input.contentsBegin);
    constexpr std::string_view occBRepToken = "DBRep_DrawableShape";
    if (matchToken(itContentsBegin, occBRepToken))
        return Format_OCCBREP;

    return Format_Unknown;
}

Format probeFormat_STL(const System::FormatProbeInput& input)
{
    const QByteArray& sample = input.contentsBegin;
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
                    | (bytes[offset+3] << 24);
            constexpr unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
            if ((facetSize * facetsCount + binaryStlHeaderSize) == input.hintFullSize)
                return Format_STL;
        }
    }

    // ASCII STL ?
    {
        // regex : ^\s*solid
        constexpr std::string_view asciiStlToken = "solid";
        auto itContentsBegin = findFirstNonSpace(input.contentsBegin);
        if (matchToken(itContentsBegin, asciiStlToken))
            return Format_STL;
    }

    return Format_Unknown;
}

Format probeFormat_OBJ(const System::FormatProbeInput& input)
{
    const QByteArray& sample = input.contentsBegin;
    const std::regex rx("^\\s*(v|vt|vn|vp|surf)\\s+[-\\+]?[0-9\\.]+\\s");
    if (std::regex_search(sample.cbegin(), sample.cend(), rx))
        return Format_OBJ;

    return Format_Unknown;
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
}

} // namespace IO
} // namespace Mayo
