/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "application_ptr.h"
#include "document.h"
#include "occ_handle.h"
#include "signal.h"
#include "span.h"

#include <CDF_DirectoryIterator.hxx>
#include <Standard_Version.hxx>
#include <XCAFApp_Application.hxx>

namespace Mayo {

// Provides management of Document objects
class Application : public XCAFApp_Application {
public:
    Application();
    ~Application();

    // Iterator over Documents contained in an Application
    struct DocumentIterator : private CDF_DirectoryIterator {
        DocumentIterator(const ApplicationPtr& app);
        DocumentIterator(const Application* app);
        bool hasNext() const;
        void next();
        DocumentPtr current() const;
        int currentIndex() const { return m_currentIndex; }
    private:
        int m_currentIndex = 0;
    };

    int documentCount() const;
    DocumentPtr newDocument(Document::Format docFormat = Document::Format::Binary);
    DocumentPtr openDocument(const FilePath& filepath, PCDM_ReaderStatus* ptrReadStatus = nullptr);
    DocumentPtr findDocumentByIndex(int docIndex) const;
    DocumentPtr findDocumentByIdentifier(Document::Identifier docIdent) const;
    DocumentPtr findDocumentByLocation(const FilePath& location) const;
    int findIndexOfDocument(const DocumentPtr& doc) const;

    void closeDocument(const DocumentPtr& doc);

    static void defineMayoFormat(const ApplicationPtr& app);

    static Span<const char*> envOpenCascadeOptions();
    static Span<const char*> envOpenCascadePaths();

    // Signals
    Signal<const DocumentPtr&> signalDocumentAdded;
    Signal<const DocumentPtr&> signalDocumentAboutToClose;
    Signal<const DocumentPtr&> signalDocumentClosed;
    Signal<const DocumentPtr&, const std::string&> signalDocumentNameChanged;
    Signal<const DocumentPtr&, const FilePath&> signalDocumentFilePathChanged;
    Signal<const DocumentPtr&, TreeNodeId> signalDocumentEntityAdded;
    Signal<const DocumentPtr&, TreeNodeId> signalDocumentEntityAboutToBeDestroyed;

public: // -- from TDocStd_Application
#if OCC_VERSION_HEX >= 0x070600
    void NewDocument(const TCollection_ExtendedString& format, OccHandle<CDM_Document>& outDoc) override;
    void InitDocument(const OccHandle<CDM_Document>& doc) const override;
#else
    void NewDocument(const TCollection_ExtendedString& format, OccHandle<TDocStd_Document>& outDoc) override;
    void InitDocument(const OccHandle<TDocStd_Document>& doc) const override;
#endif

// TODO: Redefine TDocStd_Document::BeforeClose() to emit signal documentClosed
// class Document : public TDocStd_Document { ... };
// using DocumentPtr = OccHandle<Document>
// -> Can't do because PCDM_RetrievalDriver subclasses create explicitly "new TDocStd_Document(...)"
//    This would break TDocStd_Application::Open(...)

    DEFINE_STANDARD_RTTI_INLINE(Application, XCAFApp_Application)

private: // Implementation
    friend class Document;

    void notifyDocumentAboutToClose(Document::Identifier docIdent);
    void addDocument(const DocumentPtr& doc);

    struct Private;
    Private* const d = nullptr;
};

} // namespace Mayo
