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
#include <common/mayo_version.h>

#include <QtQml/QJSEngine>

namespace Mayo {

ScriptApplication::ScriptApplication(const ApplicationPtr& app, QJSEngine* jsEngine)
    : QObject(jsEngine),
      m_jsEngine(jsEngine),
      m_app(app)
{
    if (app) {
        m_sigConns
            << app->signalDocumentAdded.connectSlot(&ScriptApplication::onDocumentAdded, this)
            << app->signalDocumentAboutToClose.connectSlot(&ScriptApplication::onDocumentAboutToClose, this)
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

QObject* ScriptApplication::newDocument()
{
    if (!m_app)
        return nullptr;

    auto doc = m_app->newDocument();
    auto jsDoc = new ScriptDocument(doc, this);
    m_vecJsDoc.push_back(jsDoc);
    m_mapIdToScriptDocument.insert({ doc->identifier(), jsDoc });
    emit this->documentAdded(jsDoc);
    emit this->documentCountChanged();
    return jsDoc;
}

QObject* ScriptApplication::documentAt(int docIndex) const
{
    if (0 <= docIndex && docIndex < m_vecJsDoc.size())
        return m_vecJsDoc.at(docIndex);
    else
        return nullptr;
}

QObject* ScriptApplication::findDocumentByLocation(const QString& location) const
{
    if (!m_app)
        return nullptr;

    auto doc = m_app->findDocumentByLocation(filepathFrom(location));
    return CppUtils::findValue(doc ? doc->identifier() : -1, m_mapIdToScriptDocument);
}

int ScriptApplication::findIndexOfDocument(QObject* doc) const
{
    auto jsDoc = qobject_cast<ScriptDocument*>(doc);
    if (!m_app || !jsDoc)
        return -1;

    return m_app->findIndexOfDocument(jsDoc->baseDocument());
}

void ScriptApplication::closeDocument(QObject* doc)
{
    auto jsDoc = qobject_cast<ScriptDocument*>(doc);
    if (m_app && jsDoc)
        m_app->closeDocument(jsDoc->baseDocument());
}

void ScriptApplication::onDocumentAdded(const DocumentPtr& doc)
{
    auto jsDoc = new ScriptDocument(doc, this);
    m_vecJsDoc.push_back(jsDoc);
    m_mapIdToScriptDocument.insert({ doc->identifier(), jsDoc });
}

void ScriptApplication::onDocumentAboutToClose(const DocumentPtr& doc)
{
    auto jsDoc = CppUtils::findValue(doc ? doc->identifier() : -1, m_mapIdToScriptDocument);
    if (jsDoc) {
        emit this->documentAboutToClose(jsDoc);
        m_mapIdToScriptDocument.erase(doc->identifier());
        m_vecJsDoc.erase(std::find(m_vecJsDoc.begin(), m_vecJsDoc.end(), jsDoc));
        jsDoc->deleteLater();
        emit this->documentCountChanged();
    }
}

} // namespace Mayo
