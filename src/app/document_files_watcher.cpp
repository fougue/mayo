/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_files_watcher.h"

#include "../base/application.h"
#include "../qtcommon/filepath_conv.h"

#include <QtCore/QFileSystemWatcher>

namespace Mayo {

DocumentFilesWatcher::DocumentFilesWatcher(const ApplicationPtr& app, QObject* parent)
    : QObject(parent),
      m_app(app)
{
    app->signalDocumentAdded.connectSlot(&DocumentFilesWatcher::onDocumentAdded, this);
    app->signalDocumentAboutToClose.connectSlot(&DocumentFilesWatcher::onDocumentAboutToClose, this);
    app->signalDocumentFilePathChanged.connectSlot(&DocumentFilesWatcher::onDocumentFilePathChanged, this);
}

void DocumentFilesWatcher::enable(bool on)
{
    if (m_isEnabled == on)
        return;

    m_isEnabled = on;
    if (on) {
        for (Application::DocumentIterator itDoc(m_app); itDoc.hasNext(); itDoc.next()) {
            const FilePath& docFilePath = itDoc.current()->filePath();
            const QString strDocFilePath = filepathTo<QString>(docFilePath);
            if (filepathExists(docFilePath))
                this->fileSystemWatcher()->addPath(strDocFilePath);
        }
    }
    else {
        this->destroyFileSystemWatcher();
        m_vecNonAckDocumentChanged.clear();
    }
}

bool DocumentFilesWatcher::isEnabled() const
{
    return m_isEnabled;
}

void DocumentFilesWatcher::acknowledgeDocumentFileChange(const DocumentPtr& doc)
{
    auto it = std::find(m_vecNonAckDocumentChanged.begin(), m_vecNonAckDocumentChanged.end(), doc);
    if (it != m_vecNonAckDocumentChanged.end())
        m_vecNonAckDocumentChanged.erase(it);
}

QFileSystemWatcher* DocumentFilesWatcher::fileSystemWatcher()
{
    if (!m_fileSystemWatcher) {
        m_fileSystemWatcher = new QFileSystemWatcher(this);
        QObject::connect(
            m_fileSystemWatcher, &QFileSystemWatcher::fileChanged,
            this, &DocumentFilesWatcher::onFileChanged
        );
    }

    return m_fileSystemWatcher;
}

void DocumentFilesWatcher::destroyFileSystemWatcher()
{
    delete m_fileSystemWatcher;
    m_fileSystemWatcher = nullptr;
}

void DocumentFilesWatcher::onFileChanged(const QString& strFilePath)
{
    if (m_isEnabled) {
        const FilePath docFilePath = filepathFrom(strFilePath);
        DocumentPtr doc = m_app->findDocumentByLocation(docFilePath);
        if (this->isDocumentChangeAcknowledged(doc)) {
            m_vecNonAckDocumentChanged.push_back(doc);
            this->signalDocumentFileChanged.send(doc);
        }
    }
}

bool DocumentFilesWatcher::isDocumentChangeAcknowledged(const DocumentPtr& doc) const
{
    auto it = std::find(m_vecNonAckDocumentChanged.cbegin(), m_vecNonAckDocumentChanged.cend(), doc);
    return it == m_vecNonAckDocumentChanged.cend();
}

void DocumentFilesWatcher::onDocumentFilePathChanged(const DocumentPtr&, const FilePath& fp)
{
    if (m_isEnabled) {
        if (filepathExists(fp))
            this->fileSystemWatcher()->addPath(filepathTo<QString>(fp));
    }
}

void DocumentFilesWatcher::onDocumentAdded(const DocumentPtr& doc)
{
    if (m_isEnabled) {
        if (filepathExists(doc->filePath()))
            this->fileSystemWatcher()->addPath(filepathTo<QString>(doc->filePath()));
    }
}

void DocumentFilesWatcher::onDocumentAboutToClose(const DocumentPtr& doc)
{
    if (m_isEnabled)
        this->fileSystemWatcher()->removePath(filepathTo<QString>(doc->filePath()));
}


} // namespace Mayo
