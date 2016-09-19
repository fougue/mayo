#include "application.h"

#include "document.h"
#include "document_item.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"
#include "options.h"
#include "fougtools/qttools/task/progress.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressIndicator.hxx>
#include <OSD_Path.hxx>
#include <RWStl.hxx>
#include <StlMesh_Mesh.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <Transfer_FinderProcess.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_WorkSession.hxx>

#include <gmio_core/error.h>
#include <gmio_stl/stl_error.h>
#include <gmio_stl/stl_infos.h>
#include <gmio_stl/stl_io.h>
#include <gmio_support/stream_qt.h>
#include <gmio_support/stl_occ_brep.h>
#include <gmio_support/stl_occ_mesh.h>

#include <algorithm>

namespace Mayo {

namespace Internal {

static QMutex* globalMutex()
{
    static QMutex mutex;
    return &mutex;
}

static bool gmio_qttask_is_stop_requested(void* cookie)
{
    auto progress = static_cast<const qttask::Progress*>(cookie);
    return progress != nullptr ? progress->isAbortRequested() : false;
}

static void gmio_qttask_handle_progress(
        void* cookie, intmax_t value, intmax_t maxValue)
{
    auto progress = static_cast<qttask::Progress*>(cookie);
    if (progress != nullptr && maxValue > 0) {
        const auto pctNorm = value / static_cast<double>(maxValue);
        const auto pct = qRound(pctNorm * 100);
        if (pct >= (progress->value() + 5))
            progress->setValue(pct);
    }
}

static gmio_task_iface gmio_qttask_create_task_iface(qttask::Progress* progress)
{
    gmio_task_iface task = {};
    task.cookie = progress;
    task.func_is_stop_requested = gmio_qttask_is_stop_requested;
    task.func_handle_progress = gmio_qttask_handle_progress;
    return task;
}

class OccImportProgress : public Message_ProgressIndicator
{
public:
    OccImportProgress(qttask::Progress* progress)
        : m_progress(progress)
    {
        this->SetScale(0., 100., 1.);
    }

    Standard_Boolean Show(const Standard_Boolean /*force*/) override
    {
        const Handle_TCollection_HAsciiString name = this->GetScope(1).GetName();
        if (!name.IsNull() && m_progress != nullptr)
            m_progress->setStep(QString(name->ToCString()));
        const Standard_Real pc = this->GetPosition(); // Always within [0,1]
        const int minVal = 0;
        const int maxVal = 100;
        const int val = minVal + pc * (maxVal - minVal);
        if (m_progress != nullptr)
            m_progress->setValue(val);
        return Standard_True;
    }

    Standard_Boolean UserBreak() override
    {
        return m_progress != nullptr ? m_progress->isAbortRequested() : false;
    }

private:
    qttask::Progress* m_progress = nullptr;
};

template<typename READER>
TopoDS_Shape loadShapeFromFile(
        const QString& filepath,
        IFSelect_ReturnStatus* error,
        qttask::Progress* progress)
{
    QMutexLocker locker(globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator = new OccImportProgress(progress);
    TopoDS_Shape result;

    if (!indicator.IsNull())
        indicator->NewScope(30, "Loading file");
    READER reader;
    *error = reader.ReadFile(
                const_cast<Standard_CString>(filepath.toLocal8Bit().constData()));
    if (!indicator.IsNull())
        indicator->EndScope();
    if (*error == IFSelect_RetDone) {
        if (!indicator.IsNull()) {
            reader.WS()->MapReader()->SetProgress(indicator);
            indicator->NewScope(70, "Translating file");
        }
        reader.NbRootsForTransfer();
        reader.TransferRoots();
        result = reader.OneShape();
        if (!indicator.IsNull()) {
            indicator->EndScope();
            reader.WS()->MapReader()->SetProgress(nullptr);
        }
    }
    return result;
}

static BRepShapeItem* createBRepShapeItem(
        const QString& filepath, const TopoDS_Shape& shape)
{
    auto partItem = new BRepShapeItem;
    partItem->setFilePath(filepath);
    partItem->propertyLabel.setValue(QFileInfo(filepath).fileName());
    partItem->setBRepShape(shape);
    return partItem;
}

static StlMeshItem* createStlMeshItem(
        const QString& filepath, const Handle_StlMesh_Mesh& mesh)
{
    auto partItem = new StlMeshItem;
    partItem->setFilePath(filepath);
    partItem->propertyLabel.setValue(QFileInfo(filepath).fileName());
    partItem->setStlMesh(mesh);
    return partItem;
}

static QString gmioErrorToQString(int error)
{
    switch (error) {
    // Core
    case GMIO_ERROR_OK:
        return QString();
    case GMIO_ERROR_UNKNOWN:
        return Application::tr("GMIO_ERROR_UNKNOWN");
    case GMIO_ERROR_NULL_MEMBLOCK:
        return Application::tr("GMIO_ERROR_NULL_MEMBLOCK");
    case GMIO_ERROR_INVALID_MEMBLOCK_SIZE:
        return Application::tr("GMIO_ERROR_INVALID_MEMBLOCK_SIZE");
    case GMIO_ERROR_STREAM:
        return Application::tr("GMIO_ERROR_STREAM");
    case GMIO_ERROR_TRANSFER_STOPPED:
        return Application::tr("GMIO_ERROR_TRANSFER_STOPPED");
    case GMIO_ERROR_STDIO:
        return Application::tr("GMIO_ERROR_STDIO");
    case GMIO_ERROR_BAD_LC_NUMERIC:
        return Application::tr("GMIO_ERROR_BAD_LC_NUMERIC");
    // STL
    case GMIO_STL_ERROR_UNKNOWN_FORMAT:
        return Application::tr("GMIO_STL_ERROR_UNKNOWN_FORMAT");
    case GMIO_STL_ERROR_NULL_FUNC_GET_TRIANGLE:
        return Application::tr("GMIO_STL_ERROR_NULL_FUNC_GET_TRIANGLE");
    case GMIO_STL_ERROR_PARSING:
        return Application::tr("GMIO_STL_ERROR_PARSING");
    case GMIO_STL_ERROR_INVALID_FLOAT32_PREC:
        return Application::tr("GMIO_STL_ERROR_INVALID_FLOAT32_PREC");
    case GMIO_STL_ERROR_UNSUPPORTED_BYTE_ORDER:
        return Application::tr("GMIO_STL_ERROR_UNSUPPORTED_BYTE_ORDER");
    case GMIO_STL_ERROR_HEADER_WRONG_SIZE:
        return Application::tr("GMIO_STL_ERROR_HEADER_WRONG_SIZE");
    case GMIO_STL_ERROR_FACET_COUNT:
        return Application::tr("GMIO_STL_ERROR_FACET_COUNT");
    }
    return Application::tr("GMIO_ERROR_UNKNOWN");
}

} // namespace Internal

Application::Application(QObject *parent)
    : QObject(parent)
{
}

Application *Application::instance()
{
    static Application app;
    return &app;
}

const std::vector<Document *> &Application::documents() const
{
    return m_documents;
}

Document *Application::addDocument(const QString &label)
{
    static unsigned docSequenceId = 0;
    auto doc = new Document(this);
    if (label.isEmpty()) {
        doc->setLabel(tr("Anonymous%1")
                      .arg(docSequenceId > 0 ?
                               QString::number(docSequenceId) :
                               QString()));
    }
    QObject::connect(
                doc, &Document::itemAdded,
                this, &Application::documentItemAdded);
    QObject::connect(
                doc, &Document::itemPropertyChanged,
                this, &Application::documentItemPropertyChanged);
    m_documents.emplace_back(doc);
    ++docSequenceId;
    emit documentAdded(doc);
    return doc;
}

bool Application::eraseDocument(Document *doc)
{
    auto itFound = std::find(m_documents.cbegin(), m_documents.cend(), doc);
    if (itFound != m_documents.cend()) {
        m_documents.erase(itFound);
        delete doc;
        emit documentErased(doc);
        return true;
    }
    return false;
}

bool Application::importInDocument(
        Document* doc,
        PartFormat format,
        const QString &filepath,
        qttask::Progress* progress)
{
    progress->setStep(QFileInfo(filepath).fileName());
    switch (format) {
    case PartFormat::Iges: return this->importIges(doc, filepath, progress);
    case PartFormat::Step: return this->importStep(doc, filepath, progress);
    case PartFormat::OccBrep: return this->importOccBRep(doc, filepath, progress);
    case PartFormat::Stl: return this->importStl(doc, filepath, progress);
    case PartFormat::Unknown: break;
    }
    return false;
}

Application::IoResult Application::exportDocumentItems(
        const std::vector<DocumentItem*> &docItems,
        PartFormat format,
        const ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
    progress->setStep(QFileInfo(filepath).fileName());
    switch (format) {
    case PartFormat::Iges:
        return this->exportIges(docItems, options, filepath, progress);
    case PartFormat::Step:
        return this->exportStep(docItems, options, filepath, progress);
    case PartFormat::OccBrep:
        return this->exportOccBRep(docItems, options, filepath, progress);
    case PartFormat::Stl:
        return this->exportStl(docItems, options, filepath, progress);
    case PartFormat::Unknown:
        break;
    }
    return { false, tr("Unknown error") };
}

bool Application::hasExportOptionsForFormat(Application::PartFormat format)
{
    return format == PartFormat::Stl;
}

const std::vector<Application::PartFormat> &Application::partFormats()
{
    const static std::vector<PartFormat> vecFormat = {
        PartFormat::Iges,
        PartFormat::Step,
        PartFormat::OccBrep,
        PartFormat::Stl
    };
    return vecFormat;
}

QString Application::partFormatFilter(Application::PartFormat format)
{
    switch (format) {
    case PartFormat::Iges: return tr("IGES files(*.iges *.igs)");
    case PartFormat::Step: return tr("STEP files(*.step *.stp)");
    case PartFormat::OccBrep: return tr("OpenCascade BREP files(*.brep *.occ)");
    case PartFormat::Stl: return tr("STL files(*.stl *.stla)");
    case PartFormat::Unknown: break;
    }
    return QString();
}

QStringList Application::partFormatFilters()
{
    QStringList filters;
    filters << Application::partFormatFilter(PartFormat::Iges)
            << Application::partFormatFilter(PartFormat::Step)
            << Application::partFormatFilter(PartFormat::OccBrep)
            << Application::partFormatFilter(PartFormat::Stl);
    return filters;
}

bool Application::importIges(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    IFSelect_ReturnStatus error;
    const TopoDS_Shape shape =
            Internal::loadShapeFromFile<IGESControl_Reader>(
                filepath, &error, progress);
    if (error == IFSelect_RetDone)
        doc->addItem(Internal::createBRepShapeItem(filepath, shape));
    return error == IFSelect_RetDone;
}

bool Application::importStep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    IFSelect_ReturnStatus error;
    const TopoDS_Shape shape =
            Internal::loadShapeFromFile<STEPControl_Reader>(
                filepath, &error, progress);
    if (error == IFSelect_RetDone)
        doc->addItem(Internal::createBRepShapeItem(filepath, shape));
    return error == IFSelect_RetDone;
}

bool Application::importOccBRep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
    const bool ok = BRepTools::Read(
            shape, filepath.toLocal8Bit().constData(), brepBuilder, indicator);
    if (ok)
        doc->addItem(Internal::createBRepShapeItem(filepath, shape));
    return ok;
}

bool Application::importStl(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    bool ok = false;
    if (lib == Options::StlIoLibrary::Gmio) {
        QFile file(filepath);
        if (file.open(QIODevice::ReadOnly)) {
            gmio_stream stream = gmio_stream_qiodevice(&file);
            gmio_stl_read_options options = {};
            options.func_stla_get_streamsize = &gmio_stla_infos_get_streamsize;
            options.task_iface = Internal::gmio_qttask_create_task_iface(progress);
            int err = GMIO_ERROR_OK;
            while (gmio_no_error(err) && !file.atEnd()) {
                Handle_StlMesh_Mesh stlMesh = new StlMesh_Mesh;
                gmio_stl_mesh_creator_occmesh meshcreator(stlMesh);
                err = gmio_stl_read(&stream, &meshcreator, &options);
                if (gmio_no_error(err))
                    doc->addItem(Internal::createStlMeshItem(filepath, stlMesh));
            }
            ok = (err == GMIO_ERROR_OK);
        }
    }
    else if (lib == Options::StlIoLibrary::OpenCascade) {
        Handle_Message_ProgressIndicator indicator =
                new Internal::OccImportProgress(progress);
        Handle_StlMesh_Mesh stlMesh = RWStl::ReadFile(
                    OSD_Path(filepath.toLocal8Bit().constData()), indicator);
        if (!stlMesh.IsNull())
            doc->addItem(Internal::createStlMeshItem(filepath, stlMesh));
        ok = !stlMesh.IsNull();
    }
    return ok;
}

Application::IoResult Application::exportIges(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    QMutexLocker locker(Internal::globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);

    IGESControl_Controller::Init();
    IGESControl_Writer writer(
                Interface_Static::CVal("XSTEP.iges.unit"),
                Interface_Static::IVal("XSTEP.iges.writebrep.mode"));
    if (!indicator.IsNull())
        writer.TransferProcess()->SetProgress(indicator);
    for (const DocumentItem* item : docItems) {
        if (sameType<BRepShapeItem>(item)) {
            auto brepItem = static_cast<const BRepShapeItem*>(item);
            writer.AddShape(brepItem->brepShape());
        }
    }
    writer.ComputeModel();
    const Standard_Boolean ok = writer.Write(
                const_cast<Standard_CString>(filepath.toLocal8Bit().constData()));
    writer.TransferProcess()->SetProgress(nullptr);
    return { ok == Standard_True, QString() };
}

Application::IoResult Application::exportStep(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    QMutexLocker locker(Internal::globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
    STEPControl_Writer writer;
    if (!indicator.IsNull())
        writer.WS()->TransferWriter()->FinderProcess()->SetProgress(indicator);
    for (const DocumentItem* item : docItems) {
        if (sameType<BRepShapeItem>(item)) {
            auto brepItem = static_cast<const BRepShapeItem*>(item);
            writer.Transfer(brepItem->brepShape(), STEPControl_AsIs);
        }
    }
    const IFSelect_ReturnStatus status = writer.Write(
                const_cast<Standard_CString>(filepath.toLocal8Bit().constData()));
    writer.WS()->TransferWriter()->FinderProcess()->SetProgress(nullptr);

    QString errorText;
    switch (status) {
    case IFSelect_RetVoid: errorText = tr("IFSelect_RetVoid"); break;
    case IFSelect_RetDone: break;
    case IFSelect_RetError: errorText = tr("IFSelect_RetError"); break;
    case IFSelect_RetFail: errorText = tr("IFSelect_RetFail"); break;
    case IFSelect_RetStop: errorText = tr("IFSelect_RetStop"); break;
    }
    return { status == IFSelect_RetDone, errorText };
}

Application::IoResult Application::exportOccBRep(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& options,
        const QString &filepath,
        qttask::Progress *progress)
{
    // TODO
    return { false, QStringLiteral("TODO") };
}

Application::IoResult Application::exportStl(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& options,
        const QString &filepath,
        qttask::Progress *progress)
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    if (lib == Options::StlIoLibrary::Gmio)
        return this->exportStl_gmio(docItems, options, filepath, progress);
    else if (lib == Options::StlIoLibrary::OpenCascade)
        return this->exportStl_OCC(docItems, options, filepath, progress);
    return { false, tr("Unknown Error") };
}

Application::IoResult Application::exportStl_gmio(
        const std::vector<DocumentItem *> &docItems,
        const Application::ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        gmio_stream stream = gmio_stream_qiodevice(&file);
        gmio_stl_write_options gmioOptions = {};
        gmioOptions.stla_float32_format = options.stlaFloat32Format;
        gmioOptions.stla_float32_prec = options.stlaFloat32Precision;
        gmioOptions.stla_solid_name = options.stlaSolidName.c_str();
        gmioOptions.task_iface =
                Internal::gmio_qttask_create_task_iface(progress);
        for (const DocumentItem* item : docItems) {
            if (progress != nullptr) {
                progress->setStep(
                            tr("Writting item %1")
                            .arg(item->propertyLabel.value()));
            }
            int error = GMIO_ERROR_OK;
            if (sameType<BRepShapeItem>(item)) {
                auto brepItem = static_cast<const BRepShapeItem*>(item);
                const gmio_stl_mesh_occshape gmioMesh(brepItem->brepShape());
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }
            else if (sameType<StlMeshItem>(item)) {
                auto meshItem = static_cast<const StlMeshItem*>(item);
                const gmio_stl_mesh_occmesh gmioMesh(meshItem->stlMesh());
                error = gmio_stl_write(
                            options.stlFormat, &stream, &gmioMesh, &gmioOptions);
            }
            if (error != GMIO_ERROR_OK)
                return { false, Internal::gmioErrorToQString(error) };
        }
        return { true, QString() };
    }
    return { false, file.errorString() };
}

Application::IoResult Application::exportStl_OCC(
        const std::vector<DocumentItem *> &docItems,
        const Application::ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
    if (options.stlFormat != GMIO_STL_FORMAT_ASCII
            && options.stlFormat != GMIO_STL_FORMAT_BINARY_LE)
    {
        return { false, tr("Format not supported") };
    }
    if (!docItems.empty() && docItems.size() > 1)
        return { false,  tr("OpenCascade RWStl does not support multi-solids") };

    if (docItems.size() > 0) {
        const DocumentItem* item = docItems.front();
        if (sameType<BRepShapeItem>(item)) {
            auto brepItem = static_cast<const BRepShapeItem*>(item);
            StlAPI_Writer writer;
            if (options.stlFormat == GMIO_STL_FORMAT_ASCII)
                writer.ASCIIMode() = Standard_True;
            else
                writer.ASCIIMode() = Standard_False;
            const StlAPI_ErrorStatus error = writer.Write(
                        brepItem->brepShape(), filepath.toLocal8Bit().constData());
            switch (error) {
            case StlAPI_StatusOK: return { true, QString() };
            case StlAPI_MeshIsEmpty: return { false, tr("StlAPI_MeshIsEmpty") };
            case StlAPI_CannotOpenFile: return { false, tr("StlAPI_CannotOpenFile") };
            case StlAPI_WriteError: return { false, tr("StlAPI_WriteError") };
            }
        }
        else if (sameType<StlMeshItem>(item)) {
            Handle_Message_ProgressIndicator indicator =
                    new Internal::OccImportProgress(progress);
            Standard_Boolean occOk = Standard_False;
            auto meshItem = static_cast<const StlMeshItem*>(item);
            const QByteArray filepathLocal8b = filepath.toLocal8Bit();
            const OSD_Path osdFilepath(filepathLocal8b.constData());
            const Handle_StlMesh_Mesh& stlMesh = meshItem->stlMesh();
            if (options.stlFormat == GMIO_STL_FORMAT_ASCII)
                occOk = RWStl::WriteAscii(stlMesh, osdFilepath, indicator);
            else
                occOk = RWStl::WriteBinary(stlMesh, osdFilepath, indicator);
            const bool ok = occOk == Standard_True;
            return { ok, ok ? QString() : tr("Unknown error") };
        }
    }
    return { true, QString() };
}

} // namespace Mayo
