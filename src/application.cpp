/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application.h"

#include "document.h"
#include "document_item.h"
#include "caf_utils.h"
#include "xde_document_item.h"
#include "mesh_item.h"
#include "options.h"
#include "mesh_utils.h"
#include "string_utils.h"
#include "fougtools/qttools/task/progress.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

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
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

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
#include <atomic>
#include <fstream>
#include <mutex>

namespace Mayo {

namespace Internal {

static std::mutex globalMutex;

#ifdef HAVE_GMIO
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
    case GMIO_ERROR_TASK_STOPPED:
        return Application::tr("GMIO_ERROR_TASK_STOPPED");
    case GMIO_ERROR_STDIO:
        return Application::tr("GMIO_ERROR_STDIO");
    case GMIO_ERROR_BAD_LC_NUMERIC:
        return Application::tr("GMIO_ERROR_BAD_LC_NUMERIC");
    // TODO: complete other core enum values
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
#endif

class OccProgress : public Message_ProgressIndicator
{
public:
    OccProgress(qttask::Progress* progress)
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

template<typename READER> // Either IGESControl_Reader or STEPControl_Reader
TopoDS_Shape loadShapeFromFile(
        const QString& filepath,
        IFSelect_ReturnStatus* error,
        qttask::Progress* progress)
{
    std::lock_guard<std::mutex> lock(globalMutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new OccProgress(progress);
    TopoDS_Shape result;

    if (!indicator.IsNull())
        indicator->NewScope(30, "Loading file");
    READER reader;
    *error = reader.ReadFile(filepath.toLocal8Bit().constData());
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

template<typename CAF_READER> struct CafReaderTraits {};

template<> struct CafReaderTraits<IGESCAFControl_Reader> {
    using ReaderType = IGESCAFControl_Reader;
    static Handle_XSControl_WorkSession workSession(const ReaderType& reader) {
        return reader.WS();
    }
    static void setPropsMode(ReaderType*, bool) { /* N/A */ }
};

template<> struct CafReaderTraits<STEPCAFControl_Reader> {
    using ReaderType = STEPCAFControl_Reader;
    static Handle_XSControl_WorkSession workSession(const ReaderType& reader) {
        return reader.Reader().WS();
    }
    static void setPropsMode(ReaderType* reader, bool on) {
        reader->SetPropsMode(on);
    }
};

template<typename CAF_READER> // Either IGESCAFControl_Reader or STEPCAFControl_Reader
void loadCafDocumentFromFile(
        const QString& filepath,
        Handle_TDocStd_Document& doc,
        IFSelect_ReturnStatus* error,
        qttask::Progress* progress)
{
    std::lock_guard<std::mutex> lock(globalMutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new OccProgress(progress);

    if (!indicator.IsNull())
        indicator->NewScope(30, "Loading file");
    CAF_READER reader;
    reader.SetColorMode(true);
    reader.SetNameMode(true);
    reader.SetLayerMode(true);
    CafReaderTraits<CAF_READER>::setPropsMode(&reader, true);
    *error = reader.ReadFile(filepath.toLocal8Bit().constData());
    if (!indicator.IsNull())
        indicator->EndScope();
    if (*error == IFSelect_RetDone) {
        Handle_XSControl_WorkSession ws =
                CafReaderTraits<CAF_READER>::workSession(reader);
        if (!indicator.IsNull()) {
            ws->MapReader()->SetProgress(indicator);
            indicator->NewScope(70, "Translating file");
        }
        if (reader.Transfer(doc) == Standard_False)
            *error = IFSelect_RetFail;
        if (!indicator.IsNull()) {
            indicator->EndScope();
            ws->MapReader()->SetProgress(nullptr);
        }
    }
}

static TopoDS_Shape xdeDocumentWholeShape(const XdeDocumentItem* xdeDocItem)
{
    TopoDS_Shape shape;
    const std::vector<TDF_Label> vecFreeShape = xdeDocItem->topLevelFreeShapes();
    if (vecFreeShape.size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TDF_Label& label : vecFreeShape)
            builder.Add(cmpd, xdeDocItem->shape(label));
        shape = cmpd;
    }
    else if (vecFreeShape.size() == 1) {
        shape = xdeDocItem->shape(vecFreeShape.front());
    }
    return shape;
}

static MeshItem* createMeshItem(
        const QString& filepath, const Handle_Poly_Triangulation& mesh)
{
    auto partItem = new MeshItem;
    partItem->propertyLabel.setValue(QFileInfo(filepath).baseName());
    partItem->propertyNodeCount.setValue(mesh->NbNodes());
    partItem->propertyTriangleCount.setValue(mesh->NbTriangles());
    partItem->propertyVolume.setQuantity(
                occ::MeshUtils::triangulationVolume(mesh) * Quantity_CubicMillimeter);
    partItem->propertyArea.setQuantity(
                occ::MeshUtils::triangulationArea(mesh) * Quantity_SquaredMillimeter);
    partItem->setTriangulation(mesh);
    return partItem;
}

static XdeDocumentItem* createXdeDocumentItem(
        const QString& filepath, const Handle_TDocStd_Document& cafDoc)
{
    auto xdeDocItem = new XdeDocumentItem(cafDoc);
    xdeDocItem->propertyLabel.setValue(QFileInfo(filepath).baseName());

    const Handle_XCAFDoc_ShapeTool& shapeTool = xdeDocItem->shapeTool();
    const std::vector<TDF_Label> vecFreeShape = xdeDocItem->topLevelFreeShapes();
    if (vecFreeShape.size() > 1) {
        const TDF_Label asmLabel = shapeTool->NewShape();
        for (const TDF_Label& shapeLabel : vecFreeShape)
            shapeTool->AddComponent(asmLabel, shapeLabel, TopLoc_Location());
        shapeTool->UpdateAssemblies();
        xdeDocItem->rebuildAssemblyTree();
    }

    const TopoDS_Shape shape = xdeDocumentWholeShape(xdeDocItem);
    GProp_GProps system;
    BRepGProp::VolumeProperties(shape, system);
    xdeDocItem->propertyVolume.setQuantity(
                std::max(system.Mass(), 0.) * Quantity_CubicMillimeter);
    BRepGProp::SurfaceProperties(shape, system);
    xdeDocItem->propertyArea.setQuantity(
                std::max(system.Mass(), 0.) * Quantity_SquaredMillimeter);

    return xdeDocItem;
}

template<size_t N>
bool matchToken(const char* buffer, const char (&token)[N])
{
    return std::strncmp(buffer, token, N - 1) == 0;
}

Application::PartFormat findPartFormatFromContents(
        const char *contentsBegin,
        size_t contentsBeginSize,
        uint64_t fullContentsSizeHint)
{
    // -- Binary STL ?
    static const size_t binaryStlHeaderSize = 80 + sizeof(uint32_t);
    if (contentsBeginSize >= binaryStlHeaderSize) {
        const uint32_t offset = 80; // Skip header
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(contentsBegin);
        const uint32_t facetsCount =
                bytes[offset]
                | (bytes[offset+1] << 8)
                | (bytes[offset+2] << 16)
                | (bytes[offset+3] << 24);
        const unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
        if ((facetSize * facetsCount + binaryStlHeaderSize)
                == fullContentsSizeHint)
        {
            return Application::PartFormat::Stl;
        }
    }

    // -- IGES ?
    {
        // regex : ^.{72}S\s*[0-9]+\s*[\n\r\f]
        bool isIges = true;
        if (contentsBeginSize >= 80 && contentsBegin[72] == 'S') {
            for (int i = 73; i < 80 && isIges; ++i) {
                if (contentsBegin[i] != ' ' && !std::isdigit(contentsBegin[i]))
                    isIges = false;
            }
            if (isIges && (contentsBegin[80] == '\n'
                           || contentsBegin[80] == '\r'
                           || contentsBegin[80] == '\f'))
            {
                const int sVal = std::atoi(contentsBegin + 73);
                if (sVal == 1)
                    return Application::PartFormat::Iges;
            }
        }
    } // IGES

    contentsBegin = StringUtils::skipWhiteSpaces(contentsBegin, contentsBeginSize);

    // -- STEP ?
    {
        // regex : ^\s*ISO-10303-21\s*;\s*HEADER
        static const char stepIsoId[] = "ISO-10303-21";
        static const size_t stepIsoIdLen = sizeof(stepIsoId) - 1;
        static const char stepHeaderToken[] = "HEADER";
        static const size_t stepHeaderTokenLen = sizeof(stepHeaderToken) - 1;
        if (std::strncmp(contentsBegin, stepIsoId, stepIsoIdLen) == 0) {
            auto charIt = StringUtils::skipWhiteSpaces(
                        contentsBegin + stepIsoIdLen,
                        contentsBeginSize - stepIsoIdLen);
            if (*charIt == ';'
                    && (charIt - contentsBegin) < contentsBeginSize)
            {
                charIt = StringUtils::skipWhiteSpaces(
                            charIt + 1,
                            contentsBeginSize - (charIt - contentsBegin));
                if (std::strncmp(charIt, stepHeaderToken, stepHeaderTokenLen)
                        == 0)
                {
                    return Application::PartFormat::Step;
                }
            }
        }
    } // STEP

    // -- OpenCascade BREP ?
    {
        // regex : ^\s*DBRep_DrawableShape
        static const char occBRepToken[] = "DBRep_DrawableShape";
        if (matchToken(contentsBegin, occBRepToken))
            return Application::PartFormat::OccBrep;
    }

    // -- ASCII STL ?
    {
        // regex : ^\s*solid
        const char asciiStlToken[] = "solid";
        if (matchToken(contentsBegin, asciiStlToken))
            return Application::PartFormat::Stl;
    }

    // Fallback case
    return Application::PartFormat::Unknown;
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

int Application::documentCount() const
{
    return static_cast<int>(m_documents.size());
}

Document *Application::documentAt(int index) const
{
    const bool validIndex = 0 <= index && index < m_documents.size();
    return validIndex ? m_documents.at(index) : nullptr;
}

const std::vector<Document*>& Application::documents() const
{
    return m_documents;
}

Document *Application::createDocument(const QString &label)
{
    auto doc = new Document(this);
    if (label.isEmpty()) {
        static std::atomic<unsigned> docSequenceId = {};
        const unsigned docId = docSequenceId.fetch_add(1);
        const QString docIdStr = docId > 0 ? QString::number(docId) : QString();
        doc->setLabel(tr("Anonymous%1").arg(docIdStr));
    }
    else {
        doc->setLabel(label);
    }
    QObject::connect(
                doc, &Document::itemAdded,
                this, &Application::documentItemAdded);
    QObject::connect(
                doc, &Document::itemPropertyChanged,
                this, &Application::documentItemPropertyChanged);
    return doc;
}

void Application::addDocument(Document *doc)
{
    auto itFound = std::find(m_documents.cbegin(), m_documents.cend(), doc);
    if (doc != nullptr && itFound == m_documents.cend()) {
        m_documents.emplace_back(doc);
        emit documentAdded(doc);
    }
}

bool Application::eraseDocument(Document *doc)
{
    auto itFound = std::find(m_documents.cbegin(), m_documents.cend(), doc);
    if (itFound != m_documents.cend()) {
        m_documents.erase(itFound);
        doc->deleteLater();
        emit documentErased(doc);
        return true;
    }
    return false;
}

Application::ArrayDocumentConstIterator
Application::findDocumentByLocation(const QFileInfo& loc) const
{
    const QString locAbsoluteFilePath = loc.absoluteFilePath();
    auto itDocFound = std::find_if(
                m_documents.cbegin(),
                m_documents.cend(),
                [=](const Document* doc) {
        return QFileInfo(doc->filePath()).absoluteFilePath() == locAbsoluteFilePath;
    });
    return itDocFound;
}

Application::IoResult Application::importInDocument(
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
    return { false, tr("Unknown error") };
}

Application::IoResult Application::exportDocumentItems(
        const std::vector<DocumentItem*>& docItems,
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

Application::PartFormat Application::findPartFormat(const QString &filepath)
{
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
#ifdef HAVE_GMIO
        gmio_stream qtstream = gmio_stream_qiodevice(&file);
        const gmio_stl_format stlFormat = gmio_stl_format_probe(&qtstream);
        if (stlFormat != GMIO_STL_FORMAT_UNKNOWN)
            return Application::PartFormat::Stl;
#endif
        std::array<char, 2048> contentsBegin;
        contentsBegin.fill(0);
        file.read(contentsBegin.data(), contentsBegin.size());
        return Internal::findPartFormatFromContents(
                    contentsBegin.data(), contentsBegin.size(), file.size());
    }
    return PartFormat::Unknown;
}

Application::IoResult Application::importIges(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    IGESControl_Controller::Init();
    Handle_TDocStd_Document cafDoc = occ::CafUtils::createXdeDocument();
    IFSelect_ReturnStatus err;
    Internal::loadCafDocumentFromFile<IGESCAFControl_Reader>(
                filepath, cafDoc, &err, progress);
    if (err == IFSelect_RetDone)
        doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    return { err == IFSelect_RetDone, StringUtils::rawText(err) };
}

Application::IoResult Application::importStep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    Handle_TDocStd_Document cafDoc = occ::CafUtils::createXdeDocument();
    IFSelect_ReturnStatus err;
    Internal::loadCafDocumentFromFile<STEPCAFControl_Reader>(
                filepath, cafDoc, &err, progress);
    if (err == IFSelect_RetDone)
        doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    return { err == IFSelect_RetDone, StringUtils::rawText(err) };
}

Application::IoResult Application::importOccBRep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
    const bool ok = BRepTools::Read(
            shape, filepath.toLocal8Bit().constData(), brepBuilder, indicator);
    if (ok) {
        Handle_TDocStd_Document cafDoc = occ::CafUtils::createXdeDocument();
        Handle_XCAFDoc_ShapeTool shapeTool =
                XCAFDoc_DocumentTool::ShapeTool(cafDoc->Main());
        const TDF_Label labelShape = shapeTool->NewShape();
        shapeTool->SetShape(labelShape, shape);
        doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    }
    return { ok, ok ? QString() : tr("Unknown Error") };
}

Application::IoResult Application::importStl(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    Application::IoResult result = { false, QString() };
    const Options::StlIoLibrary lib =
            Options::instance()->stlIoLibrary();
    if (lib == Options::StlIoLibrary::Gmio) {
#ifdef HAVE_GMIO
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
                if (gmio_no_error(err)) {
                    const Handle_Poly_Triangulation& mesh = meshcreator.polytri();
                    doc->addRootItem(Internal::createMeshItem(filepath, mesh));
                }
            }
            result.ok = (err == GMIO_ERROR_OK);
            if (!result.ok)
                result.errorText = Internal::gmioErrorToQString(err);
        }
#endif // HAVE_GMIO
    }
    else if (lib == Options::StlIoLibrary::OpenCascade) {
        Handle_Message_ProgressIndicator indicator =
                    new Internal::OccProgress(progress);
        const Handle_Poly_Triangulation mesh = RWStl::ReadFile(
                    OSD_Path(filepath.toLocal8Bit().constData()), indicator);
        if (!mesh.IsNull())
            doc->addRootItem(Internal::createMeshItem(filepath, mesh));
        result.ok = !mesh.IsNull();
        if (!result.ok)
            result.errorText = tr("Imported STL mesh is null");
    }
    return result;
}

Application::IoResult Application::exportIges(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    std::lock_guard<std::mutex> lock(Internal::globalMutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);

    IGESControl_Controller::Init();
    IGESCAFControl_Writer writer;
    writer.SetColorMode(Standard_True);
    writer.SetNameMode(Standard_True);
    writer.SetLayerMode(Standard_True);
    if (!indicator.IsNull())
        writer.TransferProcess()->SetProgress(indicator);
    for (const DocumentItem* item : docItems) {
        if (sameType<XdeDocumentItem>(item)) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
            writer.Transfer(xdeDocItem->cafDoc());
        }
    }
    writer.ComputeModel();
    const Standard_Boolean ok = writer.Write(filepath.toLocal8Bit().constData());
    writer.TransferProcess()->SetProgress(nullptr);
    return { ok == Standard_True, QString() };
}

Application::IoResult Application::exportStep(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    std::lock_guard<std::mutex> lock(Internal::globalMutex); Q_UNUSED(lock);
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
    STEPCAFControl_Writer writer;
    if (!indicator.IsNull())
        writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(indicator);
    for (const DocumentItem* item : docItems) {
        if (sameType<XdeDocumentItem>(item)) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
            writer.Transfer(xdeDocItem->cafDoc());
        }
    }
    const IFSelect_ReturnStatus err =
            writer.Write(filepath.toLocal8Bit().constData());
    writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(nullptr);
    return { err == IFSelect_RetDone, StringUtils::rawText(err) };
}

Application::IoResult Application::exportOccBRep(
        const std::vector<DocumentItem *> &docItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    std::vector<TopoDS_Shape> vecShape;
    vecShape.reserve(docItems.size());
    for (const DocumentItem* item : docItems) {
        if (sameType<XdeDocumentItem>(item)) {
            const auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
            for (const TDF_Label& label : xdeDocItem->topLevelFreeShapes())
                vecShape.push_back(xdeDocItem->shape(label));
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

    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
    const Standard_Boolean ok =
            BRepTools::Write(shape, filepath.toLocal8Bit().constData(), indicator);
    if (ok == Standard_True)
        return { true, QString() };
    return { false, tr("Unknown Error") };
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
#ifdef HAVE_GMIO
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
                return { false, Internal::gmioErrorToQString(error) };
        }
        return { true, QString() };
    }
#endif // HAVE_GMIO
    return { false, file.errorString() };
}

Application::IoResult Application::exportStl_OCC(
        const std::vector<DocumentItem *> &docItems,
        const Application::ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
#ifdef HAVE_GMIO
    const bool isAsciiFormat = options.stlFormat == GMIO_STL_FORMAT_ASCII;
    if (options.stlFormat != GMIO_STL_FORMAT_ASCII
            && options.stlFormat != GMIO_STL_FORMAT_BINARY_LE)
    {
        return { false, tr("Format not supported") };
    }
#else
    const bool isAsciiFormat = options.stlFormat == ExportOptions::StlFormat::Ascii;
#endif
    if (docItems.size() > 1)
        return { false,  tr("OpenCascade RWStl does not support multi-solids") };

    if (docItems.size() > 0) {
        const DocumentItem* item = docItems.front();
        if (sameType<XdeDocumentItem>(item)) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
            StlAPI_Writer writer;
            writer.ASCIIMode() = isAsciiFormat;
            const TopoDS_Shape shape = Internal::xdeDocumentWholeShape(xdeDocItem);
            const Standard_Boolean ok = writer.Write(
                        shape, filepath.toLocal8Bit().constData());
            return { ok, ok ? QString() : tr("Unknown StlAPI_Writer failure") };
        }
        else if (sameType<MeshItem>(item)) {
            Handle_Message_ProgressIndicator indicator =
            new Internal::OccProgress(progress);
            Standard_Boolean occOk = Standard_False;
            auto meshItem = static_cast<const MeshItem*>(item);
            const QByteArray filepathLocal8b = filepath.toLocal8Bit();
            const OSD_Path osdFilepath(filepathLocal8b.constData());
            const Handle_Poly_Triangulation& mesh = meshItem->triangulation();
            if (isAsciiFormat)
                occOk = RWStl::WriteAscii(mesh, osdFilepath, indicator);
            else
                occOk = RWStl::WriteBinary(mesh, osdFilepath, indicator);
            const bool ok = occOk == Standard_True;
            return { ok, ok ? QString() : tr("Unknown error") };
        }
    }
    return { true, QString() };
}

} // namespace Mayo
