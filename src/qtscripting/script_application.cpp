/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_application.h"
#include "script_document.h"

#include "../base/application.h"
#include "../base/cpp_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"
#include <common/mayo_version.h>

#include <QtQml/QJSEngine>

#include <future>

namespace Mayo {

ScriptApplication::ScriptApplication(
        const ApplicationPtr& app, const ScriptEnvironment& scriptEnv, QJSEngine* jsEngine
    )
    : QObject(jsEngine),
      m_scriptEnv(scriptEnv),
      m_jsEngine(jsEngine),
      m_app(app)
{
    if (app) {
        m_sigConns
            << app->signalDocumentAdded.connectSlot(&ScriptApplication::onDocumentAdded, this)
            << app->signalDocumentAboutToClose.connectSlot(&ScriptApplication::onDocumentAboutToClose, this)
            << app->signalDocumentClosed.connectSlot(&ScriptApplication::onDocumentClosed, this)
        ;
        for (Application::DocumentIterator itDoc(app); itDoc.hasNext(); itDoc.next())
            this->onDocumentAdded(itDoc.current());
    }
}

QString ScriptApplication::versionString() const
{
    return to_QString(Mayo::strVersion);
}

int ScriptApplication::documentCount() const
{
    return m_app ? m_app->documentCount() : 0;
}

//! \brief Creates and adds new document to this application
//! \details Throws signal documentAdded() when finished
QObjectPtr_ScriptDocument ScriptApplication::newDocument()
{
    if (!m_app)
        return nullptr;

    // Make sure to call Application::newDocument() in the main thread
    std::promise<DocumentPtr> docPromise;
    std::future<DocumentPtr> docFuture = docPromise.get_future();
    QtCoreUtils::runJobOnMainThread([&]{
        m_sigConns.block(true);
        auto doc = m_app->newDocument();
        m_sigConns.block(false);
        docPromise.set_value(doc);
    });
    auto doc = docFuture.get();
    return this->mapDocument(doc);
}

//! \brief Returns the document at index `docIndex`
//! \pre `0 ≤ docIndex < documentCount`
//! \return `null` if `docIndex` is invalid
QObjectPtr_ScriptDocument ScriptApplication::documentAt(int docIndex) const
{
    if (0 <= docIndex && docIndex < m_vecJsDoc.size())
        return m_vecJsDoc.at(docIndex);
    else
        return nullptr;
}

//! \brief Returns the document which was opened from filepath `location`
//! \return `null` if no document was found at input filepath
QObjectPtr_ScriptDocument ScriptApplication::findDocumentByLocation(QString location) const
{
    if (!m_app)
        return nullptr;

    auto doc = m_app->findDocumentByLocation(filepathFrom(location));
    return CppUtils::findValue(doc ? doc->identifier() : -1, m_mapIdToScriptDocument);
}

//! \brief Returns the index of the document contained in this application
//! \return `-1` if document was not found
int ScriptApplication::findIndexOfDocument(QObjectPtr_ScriptDocument doc) const
{
    auto jsDoc = qobject_cast<ScriptDocument*>(doc);
    if (!m_app || !jsDoc)
        return -1;

    return m_app->findIndexOfDocument(jsDoc->baseDocument());
}

//! \brief Closes, destroys the document `doc`
//! \details Throws documentAboutToClose() and documentClosed() signals
void ScriptApplication::closeDocument(QObjectPtr_ScriptDocument doc)
{
    auto jsDoc = qobject_cast<ScriptDocument*>(doc);
    if (m_app && jsDoc) {
        // Make sure to call Application::closeDocument() in the main thread
        std::promise<int> closePromise;
        std::future<int> closeFuture = closePromise.get_future();
        QtCoreUtils::runJobOnMainThread([&]{
            m_sigConns.block(true);
            m_app->closeDocument(jsDoc->baseDocument());
            m_sigConns.block(false);
            closePromise.set_value(0);
        });
        closeFuture.wait();
        this->unmapDocument(jsDoc->baseDocument());
    }
}

ScriptDocument* ScriptApplication::mapDocument(const DocumentPtr& doc)
{
    auto jsDoc = new ScriptDocument(doc, this);
    m_vecJsDoc.push_back(jsDoc);
    m_mapIdToScriptDocument.insert({ doc->identifier(), jsDoc });
    emit this->documentAdded(jsDoc);
    emit this->documentCountChanged();
    return jsDoc;
}

void ScriptApplication::unmapDocument(const DocumentPtr& doc)
{
    auto jsDoc = CppUtils::findValue(doc ? doc->identifier() : -1, m_mapIdToScriptDocument);
    if (jsDoc) {
        emit this->documentClosed(jsDoc);
        m_mapIdToScriptDocument.erase(doc->identifier());
        m_vecJsDoc.erase(std::find(m_vecJsDoc.begin(), m_vecJsDoc.end(), jsDoc));
        jsDoc->deleteLater();
        emit this->documentCountChanged();
    }
}

void ScriptApplication::onDocumentAdded(const DocumentPtr& doc)
{
    this->mapDocument(doc);
}

void ScriptApplication::onDocumentAboutToClose(const DocumentPtr& doc)
{
    auto jsDoc = CppUtils::findValue(doc ? doc->identifier() : -1, m_mapIdToScriptDocument);
    if (jsDoc)
        emit this->documentAboutToClose(jsDoc);
}

void ScriptApplication::onDocumentClosed(const DocumentPtr& doc)
{
    this->unmapDocument(doc);
}

} // namespace Mayo
