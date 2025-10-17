/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/document.h"
#include "script_typedefs.h"

#include <QtCore/QObject>
#include <QtQml/QJSValue>

#include <memory>
#include <unordered_map>
#include <vector>

namespace Mayo {

class ScriptMayo;

//! \brief Container of Document objects
class ScriptApplication : public QObject {
    Q_OBJECT
    Q_PROPERTY(int documentCount READ documentCount NOTIFY documentCountChanged)
public:
    ScriptApplication(const ApplicationPtr& app, ScriptMayo* scriptMayo);

    int documentCount() const;

    Q_INVOKABLE Ptr_ScriptDocument newDocument(QString name = {});
    Q_INVOKABLE Ptr_ScriptDocument documentAt(int docIndex) const;
    Q_INVOKABLE Ptr_ScriptDocument findDocumentByLocation(QString location) const;
    Q_INVOKABLE int findIndexOfDocument(Ptr_ScriptDocument doc) const;

    Q_INVOKABLE void closeDocument(Ptr_ScriptDocument doc);

    ScriptMayo* mayoObject() const { return m_scriptMayo; }

signals:
    void documentAdded(Ptr_ScriptDocument doc);
    void documentAboutToClose(Ptr_ScriptDocument doc);
    void documentClosed(Ptr_ScriptDocument doc);
    void documentCountChanged();

private:
    ScriptDocument* mapDocument(const DocumentPtr& doc);
    void unmapDocument(const DocumentPtr& doc);

    void onDocumentAdded(const DocumentPtr& doc);
    void onDocumentAboutToClose(const DocumentPtr& doc);
    void onDocumentClosed(const DocumentPtr& doc);

    ApplicationPtr m_app;
    ScriptMayo* m_scriptMayo = nullptr;
    std::vector<ScriptDocument*> m_vecJsDoc;
    std::unordered_map<Document::Identifier, ScriptDocument*> m_mapIdToScriptDocument;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo
