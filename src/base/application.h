/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document.h"
#include <CDF_DirectoryIterator.hxx>
#include <TDocStd_Application.hxx>
#include <QtCore/QFileInfo>
#include <atomic>
#include <unordered_map>

namespace Mayo {

class Application;
DEFINE_STANDARD_HANDLE(Application, TDocStd_Application)

using ApplicationPtr = opencascade::handle<Application>;

class Settings;

class Application : public QObject, public TDocStd_Application {
    Q_OBJECT
public:
    ~Application();

    static ApplicationPtr instance();

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
    DocumentPtr openDocument(const QString& filePath, PCDM_ReaderStatus* ptrReadStatus = nullptr);
    DocumentPtr findDocumentByIndex(int docIndex) const;
    DocumentPtr findDocumentByIdentifier(Document::Identifier docIdent) const;
    DocumentPtr findDocumentByLocation(const QFileInfo& loc) const;
    int findIndexOfDocument(const DocumentPtr& doc) const;

    void closeDocument(const DocumentPtr& doc);

    Settings* settings();
    const Settings* settings() const;

    static void setOpenCascadeEnvironment(const QString& settingsFilepath);

public: //  from TDocStd_Application
    void NewDocument(
            const TCollection_ExtendedString& format,
            opencascade::handle<TDocStd_Document>& outDoc) override;
    void InitDocument(const opencascade::handle<TDocStd_Document>& doc) const override;

// TODO: Redefine TDocStd_Document::BeforeClose() to emit signal documentClosed
// class Document : public TDocStd_Document { ... };
// using DocumentPtr = Handle_Document
// -> Can't do because PCDM_RetrievalDriver subclasses create explicitly "new TDocStd_Document(...)"
//    This would break TDocStd_Application::Open(...)

    DEFINE_STANDARD_RTTI_INLINE(Application, TDocStd_Application)

signals:
    void documentAdded(const DocumentPtr& doc);
    void documentAboutToClose(const DocumentPtr& doc);
    void documentNameChanged(const DocumentPtr& doc, const QString& name);
    void documentEntityAdded(const DocumentPtr& doc, TreeNodeId entityId);
    void documentEntityAboutToBeDestroyed(const DocumentPtr& doc, TreeNodeId entityId);

private: // Implementation
    friend class Document;

    Application();
    void notifyDocumentAboutToClose(Document::Identifier docIdent);
    void addDocument(const DocumentPtr& doc);

    std::atomic<Document::Identifier> m_seqDocumentIdentifier = {};
    std::unordered_map<Document::Identifier, DocumentPtr> m_mapIdentifierDocument;
    Settings* m_settings = nullptr;
};

} // namespace Mayo
