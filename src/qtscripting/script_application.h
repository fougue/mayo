/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/document.h"

#include <QtCore/QObject>
class QJSEngine;

#include <unordered_map>
#include <vector>

namespace Mayo {

class ScriptDocument;
namespace IO { class System; }

#ifndef _MAYO_DOCGEN_
using QObjectPtr_ScriptDocument = QObject*;
#endif

//! \brief Container of document objects
class ScriptApplication : public QObject {
    Q_OBJECT
    Q_PROPERTY(int documentCount READ documentCount NOTIFY documentCountChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
public:
    ScriptApplication(
        const ApplicationPtr& app,
        const IO::System* ioSystem,
        QJSEngine* jsEngine = nullptr
    );

    QString versionString() const;

    int documentCount() const;
    Q_INVOKABLE QObjectPtr_ScriptDocument newDocument();
    Q_INVOKABLE QObjectPtr_ScriptDocument documentAt(int docIndex) const;
    Q_INVOKABLE QObjectPtr_ScriptDocument findDocumentByLocation(QString location) const;
    Q_INVOKABLE int findIndexOfDocument(QObjectPtr_ScriptDocument doc) const;
    Q_INVOKABLE void closeDocument(QObjectPtr_ScriptDocument doc);

    const IO::System* ioSystem() const { return m_ioSystem; }
    QJSEngine* jsEngine() const { return m_jsEngine; }

signals:
    void documentAdded(QObjectPtr_ScriptDocument doc);
    void documentAboutToClose(QObjectPtr_ScriptDocument doc);
    void documentClosed(QObjectPtr_ScriptDocument doc);
    void documentCountChanged();

private:
    ScriptDocument* mapDocument(const DocumentPtr& doc);
    void unmapDocument(const DocumentPtr& doc);

    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);
    void onDocumentClosed(const DocumentPtr& doc);

    ApplicationPtr m_app;
    const IO::System* m_ioSystem = nullptr;
    QJSEngine* m_jsEngine = nullptr;
    std::vector<ScriptDocument*> m_vecJsDoc;
    std::unordered_map<Document::Identifier, ScriptDocument*> m_mapIdToScriptDocument;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo
