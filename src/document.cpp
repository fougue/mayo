#include "document.h"

#include "application.h"
#include "brep_shape_item.h"
#include "stl_mesh_item.h"
#include "options.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IGESControl_Reader.hxx>
#include <Message_ProgressIndicator.hxx>
#include <OSD_Path.hxx>
#include <RWStl.hxx>
#include <StlMesh_Mesh.hxx>
#include <STEPControl_Reader.hxx>
#include <Transfer_TransientProcess.hxx>
#include <XSControl_WorkSession.hxx>

#include <gmio_core/error.h>
#include <gmio_stl/stl_io.h>
#include <gmio_support/stream_qt.h>
#include <gmio_support/stl_occ_mesh.h>

#include "fougtools/qttools/task/progress.h"

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
    partItem->setLabel(QFileInfo(filepath).fileName());
    partItem->setBRepShape(shape);
    return partItem;
}

static StlMeshItem* createStlMeshItem(
        const QString& filepath, const Handle_StlMesh_Mesh& mesh)
{
    auto partItem = new StlMeshItem;
    partItem->setFilePath(filepath);
    partItem->setLabel(QFileInfo(filepath).fileName());
    partItem->setStlMesh(mesh);
    return partItem;
}

} // namespace Internal

Document::Document(Application *app)
    : QObject(app),
      m_app(app)
{
}

Document::~Document()
{
    for (DocumentItem* item : m_rootDocumentItems)
        delete item;
}

const Application *Document::application() const
{
    return m_app;
}

Application *Document::application()
{
    return m_app;
}

const QString &Document::label() const
{
    return m_label;
}

void Document::setLabel(const QString &v)
{
    m_label = v;
}

bool Document::importIges(const QString &filepath, qttask::Progress* progress)
{
    IFSelect_ReturnStatus error;
    const TopoDS_Shape shape =
            Internal::loadShapeFromFile<IGESControl_Reader>(
                filepath, &error, progress);
    if (error == IFSelect_RetDone)
        this->addPartItem(Internal::createBRepShapeItem(filepath, shape));
    return error == IFSelect_RetDone;
}

bool Document::importStep(const QString &filepath, qttask::Progress* progress)
{
    IFSelect_ReturnStatus error;
    const TopoDS_Shape shape =
            Internal::loadShapeFromFile<STEPControl_Reader>(
                filepath, &error, progress);
    if (error == IFSelect_RetDone)
        this->addPartItem(Internal::createBRepShapeItem(filepath, shape));
    return error == IFSelect_RetDone;
}

bool Document::importOccBRep(const QString &filepath, qttask::Progress* progress)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
    const bool ok = BRepTools::Read(
            shape, filepath.toLocal8Bit().constData(), brepBuilder, indicator);
    if (ok)
        this->addPartItem(Internal::createBRepShapeItem(filepath, shape));
    return ok;
}

bool Document::importStl(const QString &filepath, qttask::Progress* progress)
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    Handle_StlMesh_Mesh stlMesh;
    bool ok = false;
    if (lib == Options::StlIoLibrary::Gmio) {
        QFile file(filepath);
        if (file.open(QIODevice::ReadOnly)) {
            stlMesh = new StlMesh_Mesh;
            gmio_stl_mesh_creator_occmesh meshcreator(stlMesh);
            gmio_stream stream = gmio_stream_qiodevice(&file);
            gmio_stl_read_options options = {};
            options.task_iface = Internal::gmio_qttask_create_task_iface(progress);
            const int err = gmio_stl_read(&stream, &meshcreator, &options);
            ok = (err == GMIO_ERROR_OK);
        }
    }
    else if (lib == Options::StlIoLibrary::OpenCascade) {
        Handle_Message_ProgressIndicator indicator =
                new Internal::OccImportProgress(progress);
        stlMesh = RWStl::ReadFile(
                    OSD_Path(filepath.toLocal8Bit().constData()), indicator);
        ok = !stlMesh.IsNull();
    }
    if (ok)
        this->addPartItem(Internal::createStlMeshItem(filepath, stlMesh));
    return ok;
}

bool Document::import(
        PartFormat format, const QString &filepath, qttask::Progress* progress)
{
    switch (format) {
    case PartFormat::Iges: return this->importIges(filepath, progress);
    case PartFormat::Step: return this->importStep(filepath, progress);
    case PartFormat::OccBrep: return this->importOccBRep(filepath, progress);
    case PartFormat::Stl: return this->importStl(filepath, progress);
    case PartFormat::Unknown: break;
    }
    return false;
}

const std::vector<Document::PartFormat> &Document::partFormats()
{
    const static std::vector<PartFormat> vecFormat = {
        PartFormat::Iges,
        PartFormat::Step,
        PartFormat::OccBrep,
        PartFormat::Stl
    };
    return vecFormat;
}

QString Document::partFormatFilter(Document::PartFormat format)
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

QStringList Document::partFormatFilters()
{
    QStringList filters;
    filters << Document::partFormatFilter(PartFormat::Iges)
            << Document::partFormatFilter(PartFormat::Step)
            << Document::partFormatFilter(PartFormat::OccBrep)
            << Document::partFormatFilter(PartFormat::Stl);
    return filters;
}

const std::vector<DocumentItem *> &Document::rootDocumentItems() const
{
    return m_rootDocumentItems;
}

bool Document::isEmpty() const
{
    return m_rootDocumentItems.empty();
}

void Document::addPartItem(PartItem* partItem)
{
    m_rootDocumentItems.push_back(partItem);
    emit itemAdded(partItem);
}

} // namespace Mayo
