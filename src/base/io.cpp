/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io.h"

#include "caf_utils.h"
#include "document.h"
#include "math_utils.h"
#include "string_utils.h"
#include "task_manager.h"
#include "task_progress.h"
#include "tkernel_utils.h"
#include <fougtools/occtools/qt_utils.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressIndicator.hxx>
#include <OSD_Path.hxx>
#include <RWStl.hxx>
#include <StlAPI_Writer.hxx>
#include <Transfer_FinderProcess.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_WorkSession.hxx>

#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <TDataXtd_Triangulation.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <RWObj_CafReader.hxx>
#endif

#ifdef HAVE_GMIO
#  include <gmio_core/error.h>
#  include <gmio_stl/stl_error.h>
#  include <gmio_stl/stl_format.h>
#  include <gmio_stl/stl_infos.h>
#  include <gmio_stl/stl_io.h>
#  include <gmio_support/stream_qt.h>
#  include <gmio_support/stl_occ_brep.h>
#  include <gmio_support/stl_occ_polytri.h>
#endif

#include <algorithm>
#include <array>
#include <future>
#include <locale>
#include <fstream>
#include <memory>
#include <mutex>
#include <regex>
#include <set>

namespace Mayo {

namespace Internal {

static std::mutex globalMutex;

#ifdef HAVE_GMIO
static bool gmio_qttask_is_stop_requested(void* cookie)
{
    auto progress = static_cast<const TaskProgress*>(cookie);
    return progress ? progress->isAbortRequested() : false;
}

static void gmio_qttask_handle_progress(
        void* cookie, intmax_t value, intmax_t maxValue)
{
    auto progress = static_cast<TaskProgress*>(cookie);
    if (progress && maxValue > 0) {
        const auto pctNorm = value / static_cast<double>(maxValue);
        const auto pct = qRound(pctNorm * 100);
        if (pct >= (progress->value() + 5))
            progress->setValue(pct);
    }
}

static gmio_task_iface gmio_qttask_create_task_iface(TaskProgress* progress)
{
    gmio_task_iface task = {};
    task.cookie = progress;
    task.func_is_stop_requested = gmio_qttask_is_stop_requested;
    task.func_handle_progress = gmio_qttask_handle_progress;
    return task;
}

static QString gmioErrorToQString(int error)
{
    switch (error) {
    // Core
    case GMIO_ERROR_OK:
        return QString();
    case GMIO_ERROR_UNKNOWN:
        return IO::tr("GMIO_ERROR_UNKNOWN");
    case GMIO_ERROR_NULL_MEMBLOCK:
        return IO::tr("GMIO_ERROR_NULL_MEMBLOCK");
    case GMIO_ERROR_INVALID_MEMBLOCK_SIZE:
        return IO::tr("GMIO_ERROR_INVALID_MEMBLOCK_SIZE");
    case GMIO_ERROR_STREAM:
        return IO::tr("GMIO_ERROR_STREAM");
    case GMIO_ERROR_TASK_STOPPED:
        return IO::tr("GMIO_ERROR_TASK_STOPPED");
    case GMIO_ERROR_STDIO:
        return IO::tr("GMIO_ERROR_STDIO");
    case GMIO_ERROR_BAD_LC_NUMERIC:
        return IO::tr("GMIO_ERROR_BAD_LC_NUMERIC");
    // TODO: complete other core enum values
    // STL
    case GMIO_STL_ERROR_UNKNOWN_FORMAT:
        return IO::tr("GMIO_STL_ERROR_UNKNOWN_FORMAT");
    case GMIO_STL_ERROR_NULL_FUNC_GET_TRIANGLE:
        return IO::tr("GMIO_STL_ERROR_NULL_FUNC_GET_TRIANGLE");
    case GMIO_STL_ERROR_PARSING:
        return IO::tr("GMIO_STL_ERROR_PARSING");
    case GMIO_STL_ERROR_INVALID_FLOAT32_PREC:
        return IO::tr("GMIO_STL_ERROR_INVALID_FLOAT32_PREC");
    case GMIO_STL_ERROR_UNSUPPORTED_BYTE_ORDER:
        return IO::tr("GMIO_STL_ERROR_UNSUPPORTED_BYTE_ORDER");
    case GMIO_STL_ERROR_HEADER_WRONG_SIZE:
        return IO::tr("GMIO_STL_ERROR_HEADER_WRONG_SIZE");
    case GMIO_STL_ERROR_FACET_COUNT:
        return IO::tr("GMIO_STL_ERROR_FACET_COUNT");
    }

    return IO::tr("GMIO_ERROR_UNKNOWN");
}
#endif

class OccProgress : public Message_ProgressIndicator {
public:
    OccProgress(TaskProgress* progress)
        : m_progress(progress)
    {
        this->SetScale(0., 100., 1.);
    }

    bool Show(const bool /*force*/) override
    {
        if (m_progress) {
            const Handle_TCollection_HAsciiString name = this->GetScope(1).GetName();
            if (!name.IsNull())
                m_progress->setStep(occ::QtUtils::fromUtf8ToQString(name->String()));

            const double pc = this->GetPosition(); // Always within [0,1]
            const int minVal = 0;
            const int maxVal = 100;
            const int val = minVal + pc * (maxVal - minVal);
            m_progress->setValue(val);
        }

        return true;
    }

    bool UserBreak() override
    {
        return m_progress ? m_progress->isAbortRequested() : false;
    }

private:
    TaskProgress* m_progress = nullptr;
};

namespace OccCafIO {

static std::mutex mutex;

Handle_XSControl_WorkSession workSession(const STEPCAFControl_Reader& reader) {
    return reader.Reader().WS();
}

Handle_XSControl_WorkSession workSession(const IGESCAFControl_Reader& reader) {
    return reader.WS();
}

void readFile_prepare(const STEPCAFControl_Reader&) {
#if 0
    // Causes crash with models "RMX-10 ASSY.STEP" and "io1-cm-214.stp"
    Interface_Static::SetIVal("read.stepcaf.subshapes.name", 1);
#endif
}

void readFile_prepare(const IGESCAFControl_Reader&) {
}

template<typename CAF_READER>
bool readFile(CAF_READER& reader, const QString& filepath, TaskProgress* /*progress*/)
{
    std::lock_guard<std::mutex> lock(OccCafIO::mutex); Q_UNUSED(lock);
    readFile_prepare(reader);
    const IFSelect_ReturnStatus error = reader.ReadFile(filepath.toLocal8Bit().constData());
    return error == IFSelect_RetDone;
}

template<typename CAF_READER>
bool transfer(CAF_READER& reader, DocumentPtr doc, TaskProgress* progress)
{
    std::lock_guard<std::mutex> lock(OccCafIO::mutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new OccProgress(progress);
    Handle_XSControl_WorkSession ws = workSession(reader);
    ws->MapReader()->SetProgress(indicator);
    auto _ = gsl::finally([&]{ ws->MapReader()->SetProgress(nullptr); });
    bool ok = false;
    doc->xcafImport([&]{
        Handle_TDocStd_Document stdDoc = doc;
        ok = reader.Transfer(stdDoc);
    });
    return ok;
}

} // namespace OccCafIO

class OccStepReader : public Reader {
public:
    OccStepReader()
    {
        m_reader.SetColorMode(true);
        m_reader.SetNameMode(true);
        m_reader.SetLayerMode(true);
        m_reader.SetPropsMode(true);
    }

    bool readFile(const QString& filepath, TaskProgress* progress) override {
        return OccCafIO::readFile(m_reader, filepath, progress);
    }

    bool transfer(DocumentPtr doc, TaskProgress* progress) override {
        return OccCafIO::transfer(m_reader, doc, progress);
    }

private:
    STEPCAFControl_Reader m_reader;
};

class OccIgesReader : public Reader {
public:
    OccIgesReader()
    {
        m_reader.SetColorMode(true);
        m_reader.SetNameMode(true);
        m_reader.SetLayerMode(true);
        //m_reader.SetPropsMode(true);
    }

    bool readFile(const QString& filepath, TaskProgress* progress) override {
        return OccCafIO::readFile(m_reader, filepath, progress);
    }

    bool transfer(DocumentPtr doc, TaskProgress* progress) override {
        return OccCafIO::transfer(m_reader, doc, progress);
    }

private:
    IGESCAFControl_Reader m_reader;
};

class OccBRepReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override
    {
        m_shape.Nullify();
        m_baseFilename = QFileInfo(filepath).baseName();
        BRep_Builder brepBuilder;
        Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
        return BRepTools::Read(m_shape, filepath.toLocal8Bit().constData(), brepBuilder, indicator);
    }

    bool transfer(DocumentPtr doc, TaskProgress* /*progress*/) override
    {
        if (m_shape.IsNull())
            return false;

        doc->xcafImport([&]{
            const Handle_XCAFDoc_ShapeTool shapeTool = doc->xcaf().shapeTool();
            const TDF_Label labelShape = shapeTool->NewShape();
            shapeTool->SetShape(labelShape, m_shape);
            CafUtils::setLabelAttrStdName(labelShape, m_baseFilename);
        });
        return true;
    }

private:
    TopoDS_Shape m_shape;
    QString m_baseFilename;
};

class OccStlReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override
    {
        Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
        m_baseFilename = QFileInfo(filepath).baseName();
        m_mesh = RWStl::ReadFile(OSD_Path(filepath.toLocal8Bit().constData()), indicator);
        return !m_mesh.IsNull();
    }

    bool transfer(DocumentPtr doc, TaskProgress* /*progress*/) override
    {
        if (m_mesh.IsNull())
            return false;

        doc->singleImport([&](TDF_Label labelNewEntity) {
            TDataXtd_Triangulation::Set(labelNewEntity, m_mesh);
            CafUtils::setLabelAttrStdName(labelNewEntity, m_baseFilename);
        });
        return true;
    }

private:
    Handle_Poly_Triangulation m_mesh;
    QString m_baseFilename;
};

#ifdef HAVE_GMIO
class GmioStlReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override
    {
        m_vecMesh.clear();
        QFile file(filepath);
        if (file.open(QIODevice::ReadOnly)) {
            gmio_stream stream = gmio_stream_qiodevice(&file);
            gmio_stl_read_options options = {};
            options.func_stla_get_streamsize = &gmio_stla_infos_probe_streamsize;
            options.task_iface = Internal::gmio_qttask_create_task_iface(progress);
            int err = GMIO_ERROR_OK;
            while (gmio_no_error(err) && !file.atEnd()) {
                gmio_stl_mesh_creator_occpolytri meshcreator;
                err = gmio_stl_read(&stream, &meshcreator, &options);
                if (gmio_no_error(err))
                    m_vecMesh.push_back(meshcreator.polytri(meshcreator.polytri()));
                else
                    return false;
            }
        }

//        if (err != GMIO_ERROR_OK)
//            return Result::error(Internal::gmioErrorToQString(err));
        return true;
    }

    bool transfer(DocumentPtr doc, TaskProgress* /*progress*/) override
    {
        if (m_vecMesh.empty())
            return false;

        for (const Handle_Poly_Triangulation& mesh : m_vecMesh) {
            doc->singleImport([&](TDF_Label labelNewEntity) {
                TDataXtd_Triangulation::Set(labelNewEntity, mesh);
            });
        }

        return true;
    }

private:
    std::vector<Handle_Poly_Triangulation> m_vecMesh;
};
#endif // HAVE_GMIO

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
class OccObjReader : public Reader {
public:
    bool readFile(const QString& filepath, TaskProgress* progress) override
    {
        m_filepath = filepath;
        return true;
    }

    bool transfer(DocumentPtr doc, TaskProgress* progress) override
    {
        m_reader.SetDocument(doc);
        Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
        bool ok = true;
        doc->xcafImport([&]{
            ok = m_reader.Perform(occ::QtUtils::toOccUtf8String(m_filepath), indicator);
        });
        return ok;
    }

private:
    QString m_filepath;
    RWObj_CafReader m_reader;
};
#endif

static TopoDS_Shape xdeDocumentWholeShape(const DocumentPtr& doc)
{
    TopoDS_Shape shape;
    const TDF_LabelSequence seqFreeShape = doc->xcaf().topLevelFreeShapes();
    if (seqFreeShape.Size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TDF_Label& label : seqFreeShape)
            builder.Add(cmpd, XCaf::shape(label));

        shape = cmpd;
    }
    else if (seqFreeShape.Size() == 1) {
        shape = XCaf::shape(seqFreeShape.First());
    }

    return shape;
}

//static MeshItem* createMeshItem(
//        const QString& filepath, const Handle_Poly_Triangulation& mesh)
//{
//    auto partItem = new MeshItem;
//    partItem->propertyLabel.setValue(QFileInfo(filepath).baseName());
//    partItem->propertyNodeCount.setValue(mesh->NbNodes());
//    partItem->propertyTriangleCount.setValue(mesh->NbTriangles());
//    partItem->propertyVolume.setQuantity(
//                MeshUtils::triangulationVolume(mesh) * Quantity_CubicMillimeter);
//    partItem->propertyArea.setQuantity(
//                MeshUtils::triangulationArea(mesh) * Quantity_SquaredMillimeter);
//    partItem->setTriangulation(mesh);
//    return partItem;
//}

//static XdeDocumentItem* createXdeDocumentItem(
//        const QString& filepath, const Handle_TDocStd_Document& cafDoc)
//{
//    auto xdeDocItem = new XdeDocumentItem(cafDoc);
//    xdeDocItem->propertyLabel.setValue(QFileInfo(filepath).baseName());

//    const TopoDS_Shape shape = xdeDocumentWholeShape(xdeDocItem);
//    GProp_GProps system;
//    BRepGProp::VolumeProperties(shape, system);
//    xdeDocItem->propertyVolume.setQuantity(
//                std::max(system.Mass(), 0.) * Quantity_CubicMillimeter);
//    BRepGProp::SurfaceProperties(shape, system);
//    xdeDocItem->propertyArea.setQuantity(
//                std::max(system.Mass(), 0.) * Quantity_SquaredMillimeter);

//    return xdeDocItem;
//}

static IO::PartFormat findPartFormatFromContents(
        std::string_view contentsBegin,
        uint64_t hintFullContentsSize)
{
    // -- Binary STL ?
    constexpr size_t binaryStlHeaderSize = 80 + sizeof(uint32_t);
    if (contentsBegin.size() >= binaryStlHeaderSize) {
        constexpr uint32_t offset = 80; // Skip header
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(contentsBegin.data());
        const uint32_t facetsCount =
                bytes[offset]
                | (bytes[offset+1] << 8)
                | (bytes[offset+2] << 16)
                | (bytes[offset+3] << 24);
        constexpr unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
        if ((facetSize * facetsCount + binaryStlHeaderSize) == hintFullContentsSize)
            return IO::PartFormat::Stl;
    }

    // -- IGES ?
    {
        // regex : ^.{72}S\s*[0-9]+\s*[\n\r\f]
        bool isIges = true;
        if (contentsBegin.size() >= 80 && contentsBegin[72] == 'S') {
            for (int i = 73; i < 80 && isIges; ++i) {
                if (contentsBegin[i] != ' ' && !std::isdigit(static_cast<unsigned char>(contentsBegin[i])))
                    isIges = false;
            }

            const char c80 = contentsBegin[80];
            if (isIges && (c80 == '\n' || c80 == '\r' || c80 == '\f')) {
                const int sVal = std::atoi(contentsBegin.data() + 73);
                if (sVal == 1)
                    return IO::PartFormat::Iges;
            }
        }
    } // IGES

    const std::locale& cLocale = std::locale::classic();
    auto fnIsSpace = [&](char c) { return std::isspace(c, cLocale); };
    auto fnMatchToken = [](std::string_view::const_iterator itBegin, std::string_view token) {
        return std::strncmp(&(*itBegin), token.data(), token.size()) == 0;
    };
    auto itContentsBegin = std::find_if_not(contentsBegin.cbegin(), contentsBegin.cend(), fnIsSpace);

    // -- STEP ?
    {
        // regex : ^\s*ISO-10303-21\s*;\s*HEADER
        constexpr std::string_view stepIsoId = "ISO-10303-21";
        constexpr std::string_view stepHeaderToken = "HEADER";
        if (fnMatchToken(itContentsBegin, stepIsoId)) {
            auto itChar = std::find_if_not(itContentsBegin + stepIsoId.size(), contentsBegin.cend(), fnIsSpace);
            if (itChar != contentsBegin.cend() && *itChar == ';') {
                itChar = std::find_if_not(itChar + 1, contentsBegin.cend(), fnIsSpace);
                if (fnMatchToken(itChar, stepHeaderToken))
                    return IO::PartFormat::Step;
            }
        }
    } // STEP

    // -- OpenCascade BREP ?
    {
        // regex : ^\s*DBRep_DrawableShape
        constexpr std::string_view occBRepToken = "DBRep_DrawableShape";
        if (fnMatchToken(itContentsBegin, occBRepToken))
            return IO::PartFormat::OccBrep;
    }

    // -- ASCII STL ?
    {
        // regex : ^\s*solid
        constexpr std::string_view asciiStlToken = "solid";
        if (fnMatchToken(itContentsBegin, asciiStlToken))
            return IO::PartFormat::Stl;
    }

    // -- OBJ ?
    {
        const std::regex rx("^\\s*(v|vt|vn|vp|surf)\\s+[-\\+]?[0-9\\.]+\\s");
        if (std::regex_search(contentsBegin.cbegin(), contentsBegin.cend(), rx))
            return IO::PartFormat::Obj;
    }

    // Fallback case
    return IO::PartFormat::Unknown;
}

} // namespace Internal

IO* IO::instance()
{
    static IO io;
    return &io;
}

Span<const IO::PartFormat> IO::partFormats()
{
    const static PartFormat arrayFormat[] = {
        PartFormat::Iges,
        PartFormat::Step,
        PartFormat::OccBrep,
        PartFormat::Stl,
        PartFormat::Obj
    };

    return arrayFormat;
}

QString IO::partFormatFilter(IO::PartFormat format)
{
    switch (format) {
    case PartFormat::Iges: return tr("IGES files(*.iges *.igs)");
    case PartFormat::Step: return tr("STEP files(*.step *.stp)");
    case PartFormat::OccBrep: return tr("OpenCascade BREP files(*.brep *.occ)");
    case PartFormat::Stl: return tr("STL files(*.stl *.stla)");
    case PartFormat::Obj: return tr("OBJ files(*.obj)");
    case PartFormat::Unknown: break;
    }

    return QString();
}

QStringList IO::partFormatFilters()
{
    QStringList filters;
    for (IO::PartFormat format : IO::partFormats())
        filters << IO::partFormatFilter(format);

    return filters;
}

IO::PartFormat IO::findPartFormat(const QString& filepath)
{
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
#ifdef HAVE_GMIO
        gmio_stream qtstream = gmio_stream_qiodevice(&file);
        const gmio_stl_format stlFormat = gmio_stl_format_probe(&qtstream);
        if (stlFormat != GMIO_STL_FORMAT_UNKNOWN)
            return IO::PartFormat::Stl;
#endif
        std::array<char, 2048> contentsBegin;
        contentsBegin.fill(0);
        file.read(contentsBegin.data(), contentsBegin.size());
        return Internal::findPartFormatFromContents(
                    std::string_view(contentsBegin.data(), contentsBegin.size()),
                    file.size());
    }

    return PartFormat::Unknown;
}

IO::StlIoLibrary IO::stlIoLibrary() const
{
    return m_stlIoLibrary;
}

void IO::setStlIoLibrary(IO::StlIoLibrary lib)
{
    m_stlIoLibrary = lib;
}

std::unique_ptr<Reader> IO::createReader(IO::PartFormat format) const
{
    switch (format) {
    case PartFormat::Iges:
        return std::make_unique<Internal::OccIgesReader>();
    case PartFormat::Step:
        return std::make_unique<Internal::OccStepReader>();
    case PartFormat::OccBrep:
        return std::make_unique<Internal::OccBRepReader>();
    case PartFormat::Stl:
        return std::make_unique<Internal::OccStlReader>();
    case PartFormat::Obj:
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
        return std::make_unique<Internal::OccObjReader>();
#endif
    case PartFormat::Unknown:
        return std::unique_ptr<Reader>();
    }

    return std::unique_ptr<Reader>();
}

bool IO::importInDocument(
        DocumentPtr doc,
        const QStringList& listFilepath,
        Messenger* messenger,
        TaskProgress* progress)
{
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
        const IO::PartFormat fileFormat = IO::findPartFormat(filepath);
        if (fileFormat == IO::PartFormat::Unknown)
            return fnReadFileError(filepath, tr("Unknown format"));

        std::unique_ptr<Reader> reader = IO::instance()->createReader(fileFormat);
        if (!reader)
            return fnReadFileError(filepath, tr("No supporting reader"));

        if (!reader->readFile(filepath, subProgress))
            return fnReadFileError(filepath, tr("File read problem"));

        return reader;
    };
    auto fnTransfer = [&](QString filepath, const ReaderPtr& reader, TaskProgress* subProgress) {
        subProgress->beginScope(60, tr("Transferring file"));
        if (reader) {
            if (!reader->transfer(doc, subProgress))
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

IO::Result IO::importInDocument(
        DocumentPtr doc,
        IO::PartFormat format,
        const QString& filepath,
        TaskProgress* progress)
{
    std::unique_ptr<Reader> reader = this->createReader(format);
    if (!reader)
        return Result::error(tr("Unknown error"));

    if (!reader->readFile(filepath, progress))
        return Result::error(tr("Read file failed"));

    if (!reader->transfer(doc))
        return Result::error(tr("Transfer failed"));

    return Result::ok();
}

IO::Result IO::exportApplicationItems(
        Span<const ApplicationItem> appItems,
        PartFormat format,
        const ExportOptions& options,
        const QString& filepath,
        TaskProgress* progress)
{
    const ExportData data = { appItems, options, filepath, progress };
    switch (format) {
    case PartFormat::Iges: return this->exportIges(data);
    case PartFormat::Step: return this->exportStep(data);
    case PartFormat::OccBrep: return this->exportOccBRep(data);
    case PartFormat::Stl: return this->exportStl(data);
    case PartFormat::Unknown:
        break;
    }
    return Result::error(tr("Unknown error"));
}

bool IO::hasExportOptionsForFormat(PartFormat format)
{
    return format == PartFormat::Stl;
}

IO::IO()
{
    IGESControl_Controller::Init();
    STEPCAFControl_Controller::Init();
}

IO::Result IO::exportIges(ExportData data)
{
    std::lock_guard<std::mutex> lock(Internal::globalMutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(data.progress);

    IGESControl_Controller::Init();
    IGESCAFControl_Writer writer;
    writer.SetColorMode(true);
    writer.SetNameMode(true);
    writer.SetLayerMode(true);
    if (!indicator.IsNull())
        writer.TransferProcess()->SetProgress(indicator);

    for (const ApplicationItem& item : data.appItems) {
        if (item.isDocument())
            writer.Transfer(item.document());
        else if (item.isDocumentTreeNode())
            writer.Transfer(item.documentTreeNode().label());
    }

    writer.ComputeModel();
    const bool ok = writer.Write(data.filepath.toLocal8Bit().constData());
    writer.TransferProcess()->SetProgress(nullptr);
    return ok ? Result::ok() : Result::error(tr("Unknown error"));
}

IO::Result IO::exportStep(ExportData data)
{
    std::lock_guard<std::mutex> lock(Internal::globalMutex); Q_UNUSED(lock);
    Interface_Static::SetIVal("write.stepcaf.subshapes.name", 1);
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(data.progress);
    STEPCAFControl_Writer writer;
    if (!indicator.IsNull())
        writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(indicator);

    for (const ApplicationItem& item : data.appItems) {
        if (item.isDocument())
            writer.Transfer(item.document());
        else if (item.isDocumentTreeNode())
            writer.Transfer(item.documentTreeNode().label());
    }

    const IFSelect_ReturnStatus err = writer.Write(data.filepath.toLocal8Bit().constData());
    writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(nullptr);
    return err == IFSelect_RetDone ? Result::ok() : Result::error(StringUtils::rawText(err));
}

IO::Result IO::exportOccBRep(ExportData data)
{
    std::vector<TopoDS_Shape> vecShape;
    vecShape.reserve(data.appItems.size());
    for (const ApplicationItem& item : data.appItems) {
        if (item.isDocument()) {
            for (const TDF_Label& label : item.document()->xcaf().topLevelFreeShapes())
                vecShape.push_back(XCaf::shape(label));
        }
        else if (item.isDocumentTreeNode()) {
            const TDF_Label labelNode = item.documentTreeNode().label();
            vecShape.push_back(XCaf::shape(labelNode));
        }
    }

    TopoDS_Shape shape;
    if (vecShape.size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TopoDS_Shape& subShape : vecShape)
            builder.Add(cmpd, subShape);

        shape = cmpd;
    }
    else if (vecShape.size() == 1) {
        shape = vecShape.front();
    }

    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(data.progress);
    if (!BRepTools::Write(shape, data.filepath.toLocal8Bit().constData(), indicator))
        return Result::error(tr("Unknown Error"));

    return Result::ok();
}

IO::Result IO::exportStl(ExportData data)
{
    if (this->stlIoLibrary() == StlIoLibrary::Gmio)
        return this->exportStl_gmio(data);
    else if (this->stlIoLibrary() == StlIoLibrary::OpenCascade)
        return this->exportStl_OCC(data);

    return Result::error(tr("Unknown Error"));
}

IO::Result IO::exportStl_gmio(ExportData data)
{
    QFile file(data.filepath);
#ifdef HAVE_GMIO
    if (file.open(QIODevice::WriteOnly)) {
        gmio_stream stream = gmio_stream_qiodevice(&file);
        gmio_stl_write_options gmioOptions = {};
        gmioOptions.stla_float32_format = options.stlaFloat32Format;
        gmioOptions.stla_float32_prec = options.stlaFloat32Precision;
        gmioOptions.stla_solid_name = options.stlaSolidName.c_str();
        gmioOptions.task_iface = Internal::gmio_qttask_create_task_iface(progress);
        for (const DocumentItem* item : docItems) {
            if (progress) {
                progress->setStep(
                            tr("Writing item %1")
                            .arg(item->propertyLabel.value()));
            }

            int error = GMIO_ERROR_OK;
            if (sameType<XdeDocumentItem>(item)) {
                auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
                const TopoDS_Shape shape = Internal::xdeDocumentWholeShape(xdeDocItem);
                const gmio_stl_mesh_occshape gmioMesh(shape);
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }
            else if (sameType<MeshItem>(item)) {
                auto meshItem = static_cast<const MeshItem*>(item);
                const gmio_stl_mesh_occpolytri gmioMesh(meshItem->triangulation());
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }

            if (error != GMIO_ERROR_OK)
                return IoResult::error(Internal::gmioErrorToQString(error));
        }

        return IoResult::ok();
    }
#endif // HAVE_GMIO

    return Result::error(file.errorString());
}

IO::Result IO::exportStl_OCC(ExportData data)
{
#ifdef HAVE_GMIO
    const bool isAsciiFormat = data.options.stlFormat == GMIO_STL_FORMAT_ASCII;
    if (data.options.stlFormat != GMIO_STL_FORMAT_ASCII
            && data.options.stlFormat != GMIO_STL_FORMAT_BINARY_LE)
    {
        return Result::error(tr("Format not supported"));
    }
#else
    const bool isAsciiFormat = data.options.stlFormat == ExportOptions::StlFormat::Ascii;
#endif
    if (data.appItems.size() > 1)
        return Result::error(tr("OpenCascade RWStl does not support multi-solids"));

    auto fnWriteShape = [=](const TopoDS_Shape& shape) {
        StlAPI_Writer writer;
        writer.ASCIIMode() = isAsciiFormat;
        if (!writer.Write(shape, data.filepath.toLocal8Bit().constData()))
            return Result::error(tr("Unknown StlAPI_Writer failure"));
        else
            return Result::ok();
    };

    auto fnWriteMesh = [=](const Handle_Poly_Triangulation& mesh) {
        Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(data.progress);
        bool ok = false;
        const QByteArray filepathLocal8b = data.filepath.toLocal8Bit();
        const OSD_Path osdFilepath(filepathLocal8b.constData());
        if (isAsciiFormat)
            ok = RWStl::WriteAscii(mesh, osdFilepath, indicator);
        else
            ok = RWStl::WriteBinary(mesh, osdFilepath, indicator);

        return ok ? Result::ok() : Result::error(tr("Unknown error"));
    };

    if (!data.appItems.empty()) {
        const ApplicationItem& item = data.appItems.at(0);
        if (item.isDocument()) {
            return fnWriteShape(Internal::xdeDocumentWholeShape(item.document()));
        }
        else if (item.isDocumentTreeNode()) {
            const TDF_Label label = item.documentTreeNode().label();
            if (XCaf::isShape(label)) {
                return fnWriteShape(XCaf::shape(label));
            }
            else {
                auto attrPolyTri = CafUtils::findAttribute<TDataXtd_Triangulation>(label);
                if (!attrPolyTri.IsNull())
                    return fnWriteMesh(attrPolyTri->Get());
            }
        }
    }

    return Result::ok();
}

} // namespace Mayo
