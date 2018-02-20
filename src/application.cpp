/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "application.h"

#include "document.h"
#include "document_item.h"
#include "caf_utils.h"
#include "xde_document_item.h"
#include "stl_mesh_item.h"
#include "options.h"
#include "mesh_utils.h"
#include "stl_mesh_random_access.h"
#include "fougtools/qttools/task/progress.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressIndicator.hxx>
#include <OSD_Path.hxx>
#include <RWStl.hxx>
#include <StlMesh_Mesh.hxx>
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

#include <gmio_core/error.h>
#include <gmio_stl/stl_error.h>
#include <gmio_stl/stl_format.h>
#include <gmio_stl/stl_infos.h>
#include <gmio_stl/stl_io.h>
#include <gmio_support/stream_qt.h>
#include <gmio_support/stl_occ_brep.h>
#include <gmio_support/stl_occ_mesh.h>

#include <algorithm>
#include <array>
#include <fstream>

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

template<typename READER> // Either IGESControl_Reader or STEPControl_Reader
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
    typedef IGESCAFControl_Reader ReaderType;
    static Handle_XSControl_WorkSession workSession(const ReaderType& reader) {
        return reader.WS();
    }
};

template<> struct CafReaderTraits<STEPCAFControl_Reader> {
    typedef STEPCAFControl_Reader ReaderType;
    static Handle_XSControl_WorkSession workSession(const ReaderType& reader) {
        return reader.Reader().WS();
    }
};

template<typename CAF_READER> // Either IGESCAFControl_Reader or STEPCAFControl_Reader
void loadCafDocumentFromFile(
        const QString& filepath,
        Handle_TDocStd_Document& doc,
        IFSelect_ReturnStatus* error,
        qttask::Progress* progress)
{
    QMutexLocker locker(globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator = new OccImportProgress(progress);

    if (!indicator.IsNull())
        indicator->NewScope(30, "Loading file");
    CAF_READER reader;
    reader.SetColorMode(Standard_True);
    reader.SetNameMode(Standard_True);
    reader.SetLayerMode(Standard_True);
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
    const TDF_LabelSequence seqTopLevelFreeShapeLabel =
            xdeDocItem->topLevelFreeShapeLabels();
    if (seqTopLevelFreeShapeLabel.Size() > 1) {
        TopoDS_Compound cmpd;
        BRep_Builder builder;
        builder.MakeCompound(cmpd);
        for (const TDF_Label& label : seqTopLevelFreeShapeLabel)
            builder.Add(cmpd, xdeDocItem->shape(label));
        shape = cmpd;
    }
    else if (seqTopLevelFreeShapeLabel.Size() == 1) {
        shape = xdeDocItem->shape(seqTopLevelFreeShapeLabel.First());
    }
    return shape;
}

static StlMeshItem* createStlMeshItem(
        const QString& filepath, const Handle_StlMesh_Mesh& mesh)
{
    auto partItem = new StlMeshItem;
    partItem->propertyLabel.setValue(QFileInfo(filepath).baseName());
    const occ::StlMeshRandomAccess meshAccess(mesh);
    partItem->propertyNodeCount.setValue(meshAccess.vertexCount());
    partItem->propertyTriangleCount.setValue(meshAccess.triangleCount());
    partItem->propertyDomainCount.setValue(meshAccess.domainCount());
    partItem->propertyVolume.setValue(occ::MeshUtils::meshVolume(meshAccess));
    partItem->propertyArea.setValue(occ::MeshUtils::meshArea(meshAccess));
    partItem->setStlMesh(mesh);
    return partItem;
}

static XdeDocumentItem* createXdeDocumentItem(
        const QString& filepath, const Handle_TDocStd_Document& cafDoc)
{
    auto xdeDocItem = new XdeDocumentItem(cafDoc);
    xdeDocItem->propertyLabel.setValue(QFileInfo(filepath).baseName());

    const Handle_XCAFDoc_ShapeTool& shapeTool = xdeDocItem->shapeTool();
    const TDF_LabelSequence seqTopLevelFreeShapeLabel =
            xdeDocItem->topLevelFreeShapeLabels();
    if (seqTopLevelFreeShapeLabel.Size() > 1) {
        const TDF_Label asmLabel = shapeTool->NewShape();
        for (const TDF_Label& shapeLabel : seqTopLevelFreeShapeLabel)
            shapeTool->AddComponent(asmLabel, shapeLabel, TopLoc_Location());
        shapeTool->UpdateAssembly(asmLabel);
    }

    const TopoDS_Shape shape = xdeDocumentWholeShape(xdeDocItem);
    GProp_GProps system;
    BRepGProp::VolumeProperties(shape, system);
    xdeDocItem->propertyVolume.setValue(std::max(system.Mass(), 0.));
    BRepGProp::SurfaceProperties(shape, system);
    xdeDocItem->propertyArea.setValue(std::max(system.Mass(), 0.));

    return xdeDocItem;
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

static QString occReturnStatusToQString(IFSelect_ReturnStatus status)
{
    switch (status) {
    case IFSelect_RetVoid: return Application::tr("IFSelect_RetVoid");
    case IFSelect_RetDone: return QString();
    case IFSelect_RetError: return Application::tr("IFSelect_RetError");
    case IFSelect_RetFail: return Application::tr("IFSelect_RetFail");
    case IFSelect_RetStop: return Application::tr("IFSelect_RetStop");
    }
    return Application::tr("IFSelect Unknown");
}

const char* skipWhiteSpaces(const char* str, std::size_t len)
{
    std::size_t pos = 0;
    while (std::isspace(str[pos]) && pos < len)
        ++pos;
    return str + pos;
}

template <std::size_t N>
bool matchToken(const char* buffer, const char (&token)[N])
{
    return std::strncmp(buffer, token, N - 1) == 0;
}

Application::PartFormat findPartFormatFromContents(
        const char *contentsBegin, std::size_t contentsBeginSize)
{
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

    contentsBegin = skipWhiteSpaces(contentsBegin, contentsBeginSize);

    // -- STEP ?
    {
        // regex : ^\s*ISO-10303-21\s*;\s*HEADER
        static const char stepIsoId[] = "ISO-10303-21";
        static const std::size_t stepIsoIdLen = sizeof(stepIsoId) - 1;
        static const char stepHeaderToken[] = "HEADER";
        static const std::size_t stepHeaderTokenLen = sizeof(stepHeaderToken) - 1;
        if (std::strncmp(contentsBegin, stepIsoId, stepIsoIdLen) == 0) {
            auto charIt = skipWhiteSpaces(
                        contentsBegin + stepIsoIdLen,
                        contentsBeginSize - stepIsoIdLen);
            if (*charIt == ';'
                    && (charIt - contentsBegin) < contentsBeginSize)
            {
                charIt = skipWhiteSpaces(
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
    else {
        doc->setLabel(label);
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
        gmio_stream qtstream = gmio_stream_qiodevice(&file);
        const gmio_stl_format stlFormat = gmio_stl_format_probe(&qtstream);
        if (stlFormat != GMIO_STL_FORMAT_UNKNOWN)
            return Application::PartFormat::Stl;
        std::array<char, 2048> contentsBegin;
        contentsBegin.fill(0);
        file.read(contentsBegin.data(), contentsBegin.size());
        return Internal::findPartFormatFromContents(
                    contentsBegin.data(), contentsBegin.size());
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
    return { err == IFSelect_RetDone, Internal::occReturnStatusToQString(err) };
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
    return { err == IFSelect_RetDone, Internal::occReturnStatusToQString(err) };
}

Application::IoResult Application::importOccBRep(
        Document* doc, const QString &filepath, qttask::Progress* progress)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
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
        QFile file(filepath);
        if (file.open(QIODevice::ReadOnly)) {
            gmio_stream stream = gmio_stream_qiodevice(&file);
            gmio_stl_read_options options = {};
            options.func_stla_get_streamsize = &gmio_stla_infos_probe_streamsize;
            options.task_iface = Internal::gmio_qttask_create_task_iface(progress);
            int err = GMIO_ERROR_OK;
            while (gmio_no_error(err) && !file.atEnd()) {
                Handle_StlMesh_Mesh stlMesh = new StlMesh_Mesh;
                gmio_stl_mesh_creator_occmesh meshcreator(stlMesh);
                err = gmio_stl_read(&stream, &meshcreator, &options);
                if (gmio_no_error(err))
                    doc->addRootItem(Internal::createStlMeshItem(filepath, stlMesh));
            }
            result.ok = (err == GMIO_ERROR_OK);
            if (!result.ok)
                result.errorText = Internal::gmioErrorToQString(err);
        }
    }
    else if (lib == Options::StlIoLibrary::OpenCascade) {
        Handle_Message_ProgressIndicator indicator =
                new Internal::OccImportProgress(progress);
        Handle_StlMesh_Mesh stlMesh = RWStl::ReadFile(
                    OSD_Path(filepath.toLocal8Bit().constData()), indicator);
        if (!stlMesh.IsNull())
            doc->addRootItem(Internal::createStlMeshItem(filepath, stlMesh));
        result.ok = !stlMesh.IsNull();
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
    QMutexLocker locker(Internal::globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);

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
    QMutexLocker locker(Internal::globalMutex()); Q_UNUSED(locker);
    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
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
    return { err == IFSelect_RetDone, Internal::occReturnStatusToQString(err) };
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
            const TDF_LabelSequence seqTopLevelFreeShapeLabel =
                    xdeDocItem->topLevelFreeShapeLabels();
            for (const TDF_Label& label : seqTopLevelFreeShapeLabel)
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

    Handle_Message_ProgressIndicator indicator =
            new Internal::OccImportProgress(progress);
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
        if (sameType<XdeDocumentItem>(item)) {
            auto xdeDocItem = static_cast<const XdeDocumentItem*>(item);
            StlAPI_Writer writer;
            writer.ASCIIMode() = options.stlFormat == GMIO_STL_FORMAT_ASCII;
            const TopoDS_Shape shape = Internal::xdeDocumentWholeShape(xdeDocItem);
            const StlAPI_ErrorStatus error = writer.Write(
                        shape, filepath.toLocal8Bit().constData());
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
