/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
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

#include <fougtools/qttools/task/progress.h>

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
    const TDF_LabelSequence seqFreeShape = xdeDocItem->topLevelFreeShapes();
    if (seqFreeShape.Size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TDF_Label& label : seqFreeShape)
            builder.Add(cmpd, XdeDocumentItem::shape(label));
        shape = cmpd;
    }
    else if (seqFreeShape.Size() == 1) {
        shape = XdeDocumentItem::shape(seqFreeShape.First());
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
                MeshUtils::triangulationVolume(mesh) * Quantity_CubicMillimeter);
    partItem->propertyArea.setQuantity(
                MeshUtils::triangulationArea(mesh) * Quantity_SquaredMillimeter);
    partItem->setTriangulation(mesh);
    return partItem;
}

static XdeDocumentItem* createXdeDocumentItem(
        const QString& filepath, const Handle_TDocStd_Document& cafDoc)
{
    auto xdeDocItem = new XdeDocumentItem(cafDoc);
    xdeDocItem->propertyLabel.setValue(QFileInfo(filepath).baseName());

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
        const char* contentsBegin,
        size_t contentsBeginSize,
        uint64_t fullContentsSizeHint)
{
    // -- Binary STL ?
    constexpr size_t binaryStlHeaderSize = 80 + sizeof(uint32_t);
    if (contentsBeginSize >= binaryStlHeaderSize) {
        constexpr uint32_t offset = 80; // Skip header
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(contentsBegin);
        const uint32_t facetsCount =
                bytes[offset]
                | (bytes[offset+1] << 8)
                | (bytes[offset+2] << 16)
                | (bytes[offset+3] << 24);
        constexpr unsigned facetSize = (sizeof(float) * 12) + sizeof(uint16_t);
        if ((facetSize * facetsCount + binaryStlHeaderSize) == fullContentsSizeHint)
            return Application::PartFormat::Stl;
    }

    // -- IGES ?
    {
        // regex : ^.{72}S\s*[0-9]+\s*[\n\r\f]
        bool isIges = true;
        if (contentsBeginSize >= 80 && contentsBegin[72] == 'S') {
            for (int i = 73; i < 80 && isIges; ++i) {
                if (contentsBegin[i] != ' '
                        && !std::isdigit(static_cast<unsigned char>(contentsBegin[i])))
                {
                    isIges = false;
                }
            }

            const char c80 = contentsBegin[80];
            if (isIges && (c80 == '\n' || c80 == '\r' || c80 == '\f')) {
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
        constexpr char stepIsoId[] = "ISO-10303-21";
        constexpr char stepHeaderToken[] = "HEADER";
        constexpr size_t stepIsoIdLen = sizeof(stepIsoId) - 1;
        constexpr size_t stepHeaderTokenLen = sizeof(stepHeaderToken) - 1;
        if (std::strncmp(contentsBegin, stepIsoId, stepIsoIdLen) == 0) {
            const char* charIt = StringUtils::skipWhiteSpaces(
                        contentsBegin + stepIsoIdLen,
                        contentsBeginSize - stepIsoIdLen);
            if (*charIt == ';' && (charIt - contentsBegin) < contentsBeginSize) {
                charIt = StringUtils::skipWhiteSpaces(
                            charIt + 1,
                            contentsBeginSize - (charIt - contentsBegin));
                if (std::strncmp(charIt, stepHeaderToken, stepHeaderTokenLen) == 0)
                    return Application::PartFormat::Step;
            }
        }
    } // STEP

    // -- OpenCascade BREP ?
    {
        // regex : ^\s*DBRep_DrawableShape
        constexpr char occBRepToken[] = "DBRep_DrawableShape";
        if (matchToken(contentsBegin, occBRepToken))
            return Application::PartFormat::OccBrep;
    }

    // -- ASCII STL ?
    {
        // regex : ^\s*solid
        constexpr char asciiStlToken[] = "solid";
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
    const bool validIndex = (0 <= index) && (index < m_documents.size());
    return validIndex ? m_documents.at(index) : nullptr;
}

Span<Document* const> Application::documents() const
{
    return m_documents;
}

void Application::addDocument(Document* doc)
{
    if (doc && this->indexOfDocument(doc) == -1) {
        QObject::connect(doc, &Document::propertyChanged, [=](Property* prop) {
            emit documentPropertyChanged(doc, prop);
        });
        QObject::connect(
                    doc, &Document::itemAdded,
                    this, &Application::documentItemAdded);
        QObject::connect(
                    doc, &Document::itemErased,
                    this, &Application::documentItemErased);
        QObject::connect(
                    doc, &Document::itemPropertyChanged,
                    this, &Application::documentItemPropertyChanged);
        m_documents.emplace_back(doc);
        doc->setParent(this);
        emit documentAdded(doc);
    }
}

bool Application::eraseDocument(Document* doc)
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

int Application::findDocumentByLocation(const QFileInfo& loc) const
{
    const QString locAbsoluteFilePath = loc.absoluteFilePath();
    auto itDocFound = std::find_if(
                m_documents.cbegin(),
                m_documents.cend(),
                [=](const Document* doc) {
        return QFileInfo(doc->filePath()).absoluteFilePath() == locAbsoluteFilePath;
    });
    return itDocFound != m_documents.cend() ? itDocFound - m_documents.cbegin() : -1;
}

int Application::indexOfDocument(const Document* doc) const
{
    auto itFound = std::find(m_documents.cbegin(), m_documents.cend(), doc);
    return itFound != m_documents.cend() ? (itFound - m_documents.cbegin()) : -1;
}

Application::IoResult Application::importInDocument(
        Document* doc,
        PartFormat format,
        const QString &filepath,
        qttask::Progress* progress)
{
    switch (format) {
    case PartFormat::Iges: return this->importIges(doc, filepath, progress);
    case PartFormat::Step: return this->importStep(doc, filepath, progress);
    case PartFormat::OccBrep: return this->importOccBRep(doc, filepath, progress);
    case PartFormat::Stl: return this->importStl(doc, filepath, progress);
    case PartFormat::Unknown: break;
    }
    return IoResult::error(tr("Unknown error"));
}

Application::IoResult Application::exportApplicationItems(
        Span<const ApplicationItem> appItems,
        PartFormat format,
        const ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
    switch (format) {
    case PartFormat::Iges:
        return this->exportIges(appItems, options, filepath, progress);
    case PartFormat::Step:
        return this->exportStep(appItems, options, filepath, progress);
    case PartFormat::OccBrep:
        return this->exportOccBRep(appItems, options, filepath, progress);
    case PartFormat::Stl:
        return this->exportStl(appItems, options, filepath, progress);
    case PartFormat::Unknown:
        break;
    }
    return IoResult::error(tr("Unknown error"));
}

bool Application::hasExportOptionsForFormat(Application::PartFormat format)
{
    return format == PartFormat::Stl;
}

Span<const Application::PartFormat> Application::partFormats()
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
    Handle_TDocStd_Document cafDoc = CafUtils::createXdeDocument();
    IFSelect_ReturnStatus err;
    Internal::loadCafDocumentFromFile<IGESCAFControl_Reader>(
                filepath, cafDoc, &err, progress);
    if (err != IFSelect_RetDone)
        return IoResult::error(StringUtils::rawText(err));
    doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    return IoResult::ok();
}

Application::IoResult Application::importStep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    Interface_Static::SetIVal("read.stepcaf.subshapes.name", 1);
    Handle_TDocStd_Document cafDoc = CafUtils::createXdeDocument();
    IFSelect_ReturnStatus err;
    Internal::loadCafDocumentFromFile<STEPCAFControl_Reader>(
                filepath, cafDoc, &err, progress);
    if (err != IFSelect_RetDone)
        return IoResult::error(StringUtils::rawText(err));
    doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    return IoResult::ok();
}

Application::IoResult Application::importOccBRep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
    const bool ok = BRepTools::Read(
            shape, filepath.toLocal8Bit().constData(), brepBuilder, indicator);
    if (!ok)
        return IoResult::error(tr("Unknown Error"));
    Handle_TDocStd_Document cafDoc = CafUtils::createXdeDocument();
    Handle_XCAFDoc_ShapeTool shapeTool =
            XCAFDoc_DocumentTool::ShapeTool(cafDoc->Main());
    const TDF_Label labelShape = shapeTool->NewShape();
    shapeTool->SetShape(labelShape, shape);
    doc->addRootItem(Internal::createXdeDocumentItem(filepath, cafDoc));
    return IoResult::ok();
}

Application::IoResult Application::importStl(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
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
            if (err != GMIO_ERROR_OK)
                return IoResult::error(Internal::gmioErrorToQString(err));
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
        else
            return IoResult::error(tr("Imported STL mesh is null"));
    }
    return IoResult::ok();
}

Application::IoResult Application::exportIges(
        Span<const ApplicationItem> appItems,
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
    for (const ApplicationItem& item : appItems) {
        if (item.isDocumentItem() && sameType<XdeDocumentItem>(item.documentItem())) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item.documentItem());
            writer.Transfer(xdeDocItem->cafDoc());
        }
        else if (item.isXdeAssemblyNode()) {
            writer.Transfer(item.xdeAssemblyNode().label());
        }
    }

    writer.ComputeModel();
    const Standard_Boolean ok = writer.Write(filepath.toLocal8Bit().constData());
    writer.TransferProcess()->SetProgress(nullptr);
    return ok ? IoResult::ok() : IoResult::error(tr("Unknown error"));
}

Application::IoResult Application::exportStep(
        Span<const ApplicationItem> appItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    std::lock_guard<std::mutex> lock(Internal::globalMutex); Q_UNUSED(lock);
    Interface_Static::SetIVal("write.stepcaf.subshapes.name", 1);
    Handle_Message_ProgressIndicator indicator = new Internal::OccProgress(progress);
    STEPCAFControl_Writer writer;
    if (!indicator.IsNull())
        writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(indicator);
    for (const ApplicationItem& item : appItems) {
        if (item.isDocumentItem() && sameType<XdeDocumentItem>(item.documentItem())) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item.documentItem());
            writer.Transfer(xdeDocItem->cafDoc());
        }
        else if (item.isXdeAssemblyNode()) {
            writer.Transfer(item.xdeAssemblyNode().label());
        }
    }

    const IFSelect_ReturnStatus err =
            writer.Write(filepath.toLocal8Bit().constData());
    writer.ChangeWriter().WS()->TransferWriter()->FinderProcess()->SetProgress(nullptr);
    return err == IFSelect_RetDone ?
                IoResult::ok() :
                IoResult::error(StringUtils::rawText(err));
}

Application::IoResult Application::exportOccBRep(
        Span<const ApplicationItem> appItems,
        const ExportOptions& /*options*/,
        const QString &filepath,
        qttask::Progress *progress)
{
    std::vector<TopoDS_Shape> vecShape;
    vecShape.reserve(appItems.size());
    for (const ApplicationItem& item : appItems) {
        if (item.isDocumentItem() && sameType<XdeDocumentItem>(item.documentItem())) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item.documentItem());
            for (const TDF_Label& label : xdeDocItem->topLevelFreeShapes())
                vecShape.push_back(XdeDocumentItem::shape(label));
        }
        else if (item.isXdeAssemblyNode()) {
            vecShape.push_back(XdeDocumentItem::shape(item.xdeAssemblyNode().label()));
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
    if (!BRepTools::Write(shape, filepath.toLocal8Bit().constData(), indicator))
        return IoResult::error(tr("Unknown Error"));
    return IoResult::ok();
}

Application::IoResult Application::exportStl(
        Span<const ApplicationItem> appItems,
        const ExportOptions& options,
        const QString &filepath,
        qttask::Progress *progress)
{
    const Options::StlIoLibrary lib = Options::instance()->stlIoLibrary();
    if (lib == Options::StlIoLibrary::Gmio)
        return this->exportStl_gmio(appItems, options, filepath, progress);
    else if (lib == Options::StlIoLibrary::OpenCascade)
        return this->exportStl_OCC(appItems, options, filepath, progress);
    return IoResult::error(tr("Unknown Error"));
}

Application::IoResult Application::exportStl_gmio(
        Span<const ApplicationItem> appItems,
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
    return IoResult::error(file.errorString());
}

Application::IoResult Application::exportStl_OCC(
        Span<const ApplicationItem> appItems,
        const Application::ExportOptions &options,
        const QString &filepath,
        qttask::Progress *progress)
{
#ifdef HAVE_GMIO
    const bool isAsciiFormat = options.stlFormat == GMIO_STL_FORMAT_ASCII;
    if (options.stlFormat != GMIO_STL_FORMAT_ASCII
            && options.stlFormat != GMIO_STL_FORMAT_BINARY_LE)
    {
        return IoResult::error(tr("Format not supported"));
    }
#else
    const bool isAsciiFormat = options.stlFormat == ExportOptions::StlFormat::Ascii;
#endif
    if (appItems.size() > 1)
        return IoResult::error(tr("OpenCascade RWStl does not support multi-solids"));

    auto funcWriteShape = [=](const TopoDS_Shape& shape) {
        StlAPI_Writer writer;
        writer.ASCIIMode() = isAsciiFormat;
        if (!writer.Write(shape, filepath.toLocal8Bit().constData()))
            return IoResult::error(tr("Unknown StlAPI_Writer failure"));
        return IoResult::ok();
    };

    if (!appItems.empty()) {
        const ApplicationItem& item = appItems.at(0);
        if (item.isDocumentItem()) {
            if (sameType<XdeDocumentItem>(item.documentItem())) {
                auto xdeDocItem = static_cast<const XdeDocumentItem*>(item.documentItem());
                return funcWriteShape(Internal::xdeDocumentWholeShape(xdeDocItem));
            }
            else if (sameType<MeshItem>(item.documentItem())) {
                Handle_Message_ProgressIndicator indicator =
                        new Internal::OccProgress(progress);
                bool ok = false;
                auto meshItem = static_cast<const MeshItem*>(item.documentItem());
                const QByteArray filepathLocal8b = filepath.toLocal8Bit();
                const OSD_Path osdFilepath(filepathLocal8b.constData());
                const Handle_Poly_Triangulation& mesh = meshItem->triangulation();
                if (isAsciiFormat)
                    ok = RWStl::WriteAscii(mesh, osdFilepath, indicator);
                else
                    ok = RWStl::WriteBinary(mesh, osdFilepath, indicator);

                if (!ok)
                    return IoResult::error(tr("Unknown error"));
            }
        }
        else if (item.isXdeAssemblyNode()) {
            const TDF_Label& labelShape = item.xdeAssemblyNode().label();
            return funcWriteShape(XdeDocumentItem::shape(labelShape));
        }
    }

    return IoResult::ok();
}

} // namespace Mayo
