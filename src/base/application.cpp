/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application.h"
#include "document_tree_node_properties_provider.h"
#include "filepath_conv.h"
#include "io_system.h"
#include "property_builtins.h"
#include "task_common.h"
#include "tkernel_utils.h"

#include <BinXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <BinXCAFDrivers_DocumentStorageDriver.hxx>
#include <XCAFApp_Application.hxx>
#include <XmlXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <XmlXCAFDrivers_DocumentStorageDriver.hxx>
#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 5, 0)
#  include <CDF_Session.hxx>
#endif

#include <atomic>
#include <vector>
#include <unordered_map>

namespace Mayo {

class Document::FormatBinaryRetrievalDriver : public BinXCAFDrivers_DocumentRetrievalDriver {
public:
    FormatBinaryRetrievalDriver(const ApplicationPtr& app) : m_app(app) {}

#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 6, 0)
    Handle(CDM_Document) CreateDocument() override { return new Document(m_app);  }
#endif

private:
    ApplicationPtr m_app;
};

class Document::FormatXmlRetrievalDriver : public XmlXCAFDrivers_DocumentRetrievalDriver {
public:
    FormatXmlRetrievalDriver(const ApplicationPtr& app) : m_app(app) {}

#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 6, 0)
    Handle(CDM_Document) CreateDocument() override { return new Document(m_app); }
#endif

private:
    ApplicationPtr m_app;
};

struct Application::Private {
    std::atomic<Document::Identifier> m_seqDocumentIdentifier = {};
    std::unordered_map<Document::Identifier, DocumentPtr> m_mapIdentifierDocument;
    DocumentTreeNodePropertiesProviderTable m_documentTreeNodePropertiesProviderTable;
    std::vector<Application::Translator> m_vecTranslator;
};

Application::~Application()
{
    delete d;
}

const ApplicationPtr& Application::instance()
{
    static ApplicationPtr appPtr;
    if (appPtr.IsNull()) {
        appPtr = new Application;
        const char strFougueCopyright[] = "Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>";
        appPtr->DefineFormat(
                    Document::NameFormatBinary, qUtf8Printable(tr("Binary Mayo Document Format")), "myb",
                    new Document::FormatBinaryRetrievalDriver(appPtr),
                    new BinXCAFDrivers_DocumentStorageDriver);
        appPtr->DefineFormat(
                    Document::NameFormatXml, qUtf8Printable(tr("XML Mayo Document Format")), "myx",
                    new Document::FormatXmlRetrievalDriver(appPtr),
                    new XmlXCAFDrivers_DocumentStorageDriver(strFougueCopyright));

        qRegisterMetaType<TreeNodeId>("Mayo::TreeNodeId");
        qRegisterMetaType<TreeNodeId>("TreeNodeId");
        qRegisterMetaType<DocumentPtr>("Mayo::DocumentPtr");
        qRegisterMetaType<DocumentPtr>("DocumentPtr");
        qRegisterMetaType<TaskId>("Mayo::TaskId");
        qRegisterMetaType<TaskId>("TaskId");
        qRegisterMetaType<std::string>("std::string");
    }

    return appPtr;
}

int Application::documentCount() const
{
    return this->NbDocuments();
}

DocumentPtr Application::newDocument(Document::Format docFormat)
{
    const char* docNameFormat = Document::toNameFormat(docFormat);
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    Handle(CDM_Document) stdDoc;
#else
    Handle(TDocStd_Document) stdDoc;
#endif
    this->NewDocument(docNameFormat, stdDoc);
    return DocumentPtr::DownCast(stdDoc);
}

DocumentPtr Application::openDocument(const FilePath& filepath, PCDM_ReaderStatus* ptrReadStatus)
{
    Handle_TDocStd_Document stdDoc;
    const PCDM_ReaderStatus readStatus = this->Open(filepathTo<TCollection_ExtendedString>(filepath), stdDoc);
    if (ptrReadStatus)
        *ptrReadStatus = readStatus;

    DocumentPtr doc = DocumentPtr::DownCast(stdDoc);
    this->addDocument(doc);
    return doc;
}

DocumentPtr Application::findDocumentByIndex(int docIndex) const
{
    Handle_TDocStd_Document doc;
    TDocStd_Application::GetDocument(docIndex + 1, doc);
    return !doc.IsNull() ? DocumentPtr::DownCast(doc) : DocumentPtr();
}

DocumentPtr Application::findDocumentByIdentifier(Document::Identifier docIdent) const
{
    auto itFound = d->m_mapIdentifierDocument.find(docIdent);
    return itFound != d->m_mapIdentifierDocument.cend() ? itFound->second : DocumentPtr();
}

DocumentPtr Application::findDocumentByLocation(const FilePath& location) const
{
    for (const auto& mapPair : d->m_mapIdentifierDocument) {
        const DocumentPtr& docPtr = mapPair.second;
        if (filepathEquivalent(docPtr->filePath(), location))
            return docPtr;
    }

    return DocumentPtr();
}

int Application::findIndexOfDocument(const DocumentPtr& doc) const
{
    for (DocumentIterator it(this); it.hasNext(); it.next()) {
        if (it.current() == doc)
            return it.currentIndex();
    }

    return -1;
}

void Application::closeDocument(const DocumentPtr& doc)
{
    TDocStd_Application::Close(doc);
}

DocumentTreeNodePropertiesProviderTable* Application::documentTreeNodePropertiesProviderTable() const
{
    return &(d->m_documentTreeNodePropertiesProviderTable);
}

void Application::addTranslator(Application::Translator fn)
{
    if (fn)
        d->m_vecTranslator.push_back(std::move(fn));
}

std::string_view Application::translate(const TextId& textId, int n) const
{
    for (auto it = d->m_vecTranslator.rbegin(); it != d->m_vecTranslator.rend(); ++it) {
        const Application::Translator& fn = *it;
        std::string_view msg = fn(textId, n);
        if (!msg.empty())
            return msg;
    }

    return textId.key;
}

Span<const char*> Application::envOpenCascadeOptions()
{
    static const char* arrayOptionName[] = {
        "MMGT_OPT",
        "MMGT_CLEAR",
        "MMGT_REENTRANT",
        "CSF_LANGUAGE",
        "CSF_EXCEPTION_PROMPT"
    };
    return arrayOptionName;
}

Span<const char*> Application::envOpenCascadePaths()
{
    static const char* arrayPathName[] = {
        "CSF_SHMessage",
        "CSF_MDTVTexturesDirectory",
        "CSF_ShadersDirectory",
        "CSF_XSMessage",
        "CSF_TObjMessage",
        "CSF_StandardDefaults",
        "CSF_PluginDefaults",
        "CSF_XCAFDefaults",
        "CSF_TObjDefaults",
        "CSF_StandardLiteDefaults",
        "CSF_IGESDefaults",
        "CSF_STEPDefaults",
        "CSF_XmlOcafResource",
        "CSF_MIGRATION_TYPES"
    };
    return arrayPathName;
}
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
void Application::NewDocument(const TCollection_ExtendedString&, Handle(CDM_Document)& outDocument)
#else
void Application::NewDocument(const TCollection_ExtendedString&, Handle(TDocStd_Document)& outDocument)
#endif
{
    // TODO: check format == "mayo" if not throw exception
    // Extended from TDocStd_Application::NewDocument() implementation, ensure that in future
    // OpenCascade versions this code is still compatible!
    DocumentPtr newDoc = new Document(this);
    CDF_Application::Open(newDoc); // Add the document in the session
    this->addDocument(newDoc);
    outDocument = newDoc;
}

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
void Application::InitDocument(const Handle(CDM_Document)& doc) const
#else
void Application::InitDocument(const Handle(TDocStd_Document)& doc) const
#endif
{
    TDocStd_Application::InitDocument(doc);
    XCAFApp_Application::GetApplication()->InitDocument(doc);
}

Application::Application()
    : QObject(nullptr),
      d(new Private)
{
}

void Application::notifyDocumentAboutToClose(Document::Identifier docIdent)
{
    auto itFound = d->m_mapIdentifierDocument.find(docIdent);
    if (itFound != d->m_mapIdentifierDocument.end()) {
        emit this->documentAboutToClose(itFound->second);
        d->m_mapIdentifierDocument.erase(itFound);
    }
}

void Application::addDocument(const DocumentPtr& doc)
{
    if (!doc.IsNull()) {
        doc->setIdentifier(d->m_seqDocumentIdentifier.fetch_add(1));
        d->m_mapIdentifierDocument.insert({ doc->identifier(), doc });
        this->InitDocument(doc);
        doc->initXCaf();

        QObject::connect(
                    doc.get(), &Document::nameChanged,
                    this, [=](const std::string& name) { emit this->documentNameChanged(doc, name); });
        QObject::connect(
                    doc.get(), &Document::entityAdded,
                    this, [=](TreeNodeId entityId) { emit this->documentEntityAdded(doc, entityId); });
        QObject::connect(
                    doc.get(), &Document::entityAboutToBeDestroyed,
                    this, [=](TreeNodeId entityId) { emit this->documentEntityAboutToBeDestroyed(doc, entityId); });
//      QObject::connect(
//                  doc, &Document::itemPropertyChanged,
//                  this, &Application::documentItemPropertyChanged);
        emit documentAdded(doc);
    }
}

Application::DocumentIterator::DocumentIterator(const ApplicationPtr& app)
    : DocumentIterator(app.get())
{
}

Application::DocumentIterator::DocumentIterator(const Application* app)
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    : CDF_DirectoryIterator(app->myDirectory)
#else
    : CDF_DirectoryIterator(CDF_Session::CurrentSession()->Directory())
#endif
{
}

bool Application::DocumentIterator::hasNext() const
{
    return const_cast<DocumentIterator*>(this)->MoreDocument();
}

void Application::DocumentIterator::next()
{
    this->NextDocument();
    ++m_currentIndex;
}

DocumentPtr Application::DocumentIterator::current() const
{
    return DocumentPtr::DownCast(const_cast<DocumentIterator*>(this)->Document());
}

} // namespace Mayo
