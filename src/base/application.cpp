/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application.h"
#include "filepath_conv.h"
#include "property_builtins.h"
#include "task_common.h"
#include "tkernel_utils.h"

#include <BinXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <BinXCAFDrivers_DocumentStorageDriver.hxx>
#include <XmlXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <XmlXCAFDrivers_DocumentStorageDriver.hxx>
#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 5, 0)
#  include <CDF_Session.hxx>
#endif

#include <atomic>
#include <unordered_map>

namespace Mayo {

class Document::FormatBinaryRetrievalDriver : public BinXCAFDrivers_DocumentRetrievalDriver {
public:
    FormatBinaryRetrievalDriver(const ApplicationPtr& app) : m_app(app) {}

#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 6, 0)
    OccHandle<CDM_Document> CreateDocument() override { return new Document(m_app);  }
#endif

private:
    ApplicationPtr m_app;
};

class Document::FormatXmlRetrievalDriver : public XmlXCAFDrivers_DocumentRetrievalDriver {
public:
    FormatXmlRetrievalDriver(const ApplicationPtr& app) : m_app(app) {}

#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 6, 0)
    OccHandle<CDM_Document> CreateDocument() override { return new Document(m_app); }
#endif

private:
    ApplicationPtr m_app;
};

struct Application::Private {
    std::atomic<Document::Identifier> m_seqDocumentIdentifier = {};
    std::unordered_map<Document::Identifier, DocumentPtr> m_mapIdentifierDocument;
};

struct ApplicationI18N {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Application)
};

Application::Application()
    : d(new Private)
{
}

Application::~Application()
{
    delete d;
}

int Application::documentCount() const
{
    return this->NbDocuments();
}

DocumentPtr Application::newDocument(Document::Format docFormat)
{
    const char* docNameFormat = Document::toNameFormat(docFormat);
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
    OccHandle<CDM_Document> stdDoc;
#else
    OccHandle<TDocStd_Document> stdDoc;
#endif
    this->NewDocument(docNameFormat, stdDoc);
    return DocumentPtr::DownCast(stdDoc);
}

DocumentPtr Application::openDocument(const FilePath& filepath, PCDM_ReaderStatus* ptrReadStatus)
{
    OccHandle<TDocStd_Document> stdDoc;
    const PCDM_ReaderStatus readStatus = this->Open(filepathTo<TCollection_ExtendedString>(filepath), stdDoc);
    if (ptrReadStatus)
        *ptrReadStatus = readStatus;

    DocumentPtr doc = DocumentPtr::DownCast(stdDoc);
    this->addDocument(doc);
    return doc;
}

DocumentPtr Application::findDocumentByIndex(int docIndex) const
{
    OccHandle<TDocStd_Document> doc;
    XCAFApp_Application::GetDocument(docIndex + 1, doc);
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
    if (!doc)
        return;

    XCAFApp_Application::Close(doc);
    doc->signalNameChanged.disconnectAll();
    doc->signalFilePathChanged.disconnectAll();
    doc->signalEntityAdded.disconnectAll();
    doc->signalEntityAboutToBeDestroyed.disconnectAll();
    this->signalDocumentClosed.send(doc);
    //doc->Main().ForgetAllAttributes(true/*clearChildren*/);
}

void Application::defineMayoFormat(const ApplicationPtr& app)
{
    const char strFougueCopyright[] = "Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>";
    app->DefineFormat(
        Document::NameFormatBinary, ApplicationI18N::textIdTr("Binary Mayo Document Format").data(), "myb",
        new Document::FormatBinaryRetrievalDriver(app),
        new BinXCAFDrivers_DocumentStorageDriver
    );
    app->DefineFormat(
        Document::NameFormatXml, ApplicationI18N::textIdTr("XML Mayo Document Format").data(), "myx",
        new Document::FormatXmlRetrievalDriver(app),
        new XmlXCAFDrivers_DocumentStorageDriver(strFougueCopyright)
    );
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
void Application::NewDocument(const TCollection_ExtendedString&, OccHandle<CDM_Document>& outDocument)
#else
void Application::NewDocument(const TCollection_ExtendedString&, OccHandle<TDocStd_Document>& outDocument)
#endif
{
    // TODO: check format == "mayo" if not throw exception
    // Extended from XCAFApp_Application::NewDocument() implementation, ensure that in future
    // OpenCascade versions this code is still compatible!
    DocumentPtr newDoc = new Document(this);
    CDF_Application::Open(newDoc); // Add the document in the session
    this->addDocument(newDoc);
    outDocument = newDoc;
}

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 6, 0)
void Application::InitDocument(const OccHandle<CDM_Document>& doc) const
#else
void Application::InitDocument(const OccHandle<TDocStd_Document>& doc) const
#endif
{
    XCAFApp_Application::InitDocument(doc);
}

void Application::notifyDocumentAboutToClose(Document::Identifier docIdent)
{
    auto itFound = d->m_mapIdentifierDocument.find(docIdent);
    if (itFound != d->m_mapIdentifierDocument.end()) {
        this->signalDocumentAboutToClose.send(itFound->second);
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

        doc->signalNameChanged.connectSlot([=](const std::string& name) {
            this->signalDocumentNameChanged.send(doc, name);
        });
        doc->signalFilePathChanged.connectSlot([=](const FilePath& fp) {
            this->signalDocumentFilePathChanged.send(doc, fp);
        });
        doc->signalEntityAdded.connectSlot([=](TreeNodeId entityId) {
            this->signalDocumentEntityAdded.send(doc, entityId);
        });
        doc->signalEntityAboutToBeDestroyed.connectSlot([=](TreeNodeId entityId) {
            this->signalDocumentEntityAboutToBeDestroyed.send(doc, entityId);
        });
        this->signalDocumentAdded.send(doc);
    }
}

Application::DocumentIterator::DocumentIterator(const ApplicationPtr& app)
    : DocumentIterator(app.get())
{
}

Application::DocumentIterator::DocumentIterator([[maybe_unused]] const Application* app)
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
