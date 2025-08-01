/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "script_global.h"
#include "../base/application_ptr.h"
#include "../base/document.h"
#include "../base/messenger.h"
#include "../base/task_manager.h"

#include <QtCore/QObject>
#include <QtQml/QJSValue>
class QJSEngine;

#include <memory>
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
        const ScriptEnvironment& scriptEnv,
        QJSEngine* jsEngine = nullptr
    );

    QString versionString() const;

    int documentCount() const;
    Q_INVOKABLE QObjectPtr_ScriptDocument newDocument();
    Q_INVOKABLE QObjectPtr_ScriptDocument documentAt(int docIndex) const;
    Q_INVOKABLE QObjectPtr_ScriptDocument findDocumentByLocation(QString location) const;
    Q_INVOKABLE int findIndexOfDocument(QObjectPtr_ScriptDocument doc) const;
    Q_INVOKABLE void closeDocument(QObjectPtr_ScriptDocument doc);

    Q_INVOKABLE bool waitForDone(int msecs = -1);
    Q_INVOKABLE bool waitForDone(quint32 taskId, int msecs = -1);
    Q_INVOKABLE bool waitForDone(QJSValueList taskIdList, int msecs = -1);

    const ScriptEnvironment& environment() const { return m_scriptEnv; }
    QJSEngine* jsEngine() const { return m_jsEngine; }
    TaskManager& taskManager() { return m_taskMgr; }

    struct TaskCallbacks {
        QJSValue onStarted;
        QJSValue onProgress;
        QJSValue onEnded;
        QJSValue onInfo;
        QJSValue onWarning;
        QJSValue onError;
    };
    void registerTask(
        TaskId taskId, const TaskCallbacks& callbacks, std::unique_ptr<MessengerBySignal> msg
    );

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

    struct Task {
        int progressPct = 0;
        QString progressStepTitle;
        TaskCallbacks callbacks;
        std::unique_ptr<MessengerBySignal> messenger;
    };
    Task* findTask(TaskId taskId);

    ApplicationPtr m_app;
    const ScriptEnvironment m_scriptEnv;
    QJSEngine* m_jsEngine = nullptr;
    std::vector<ScriptDocument*> m_vecJsDoc;
    std::unordered_map<Document::Identifier, ScriptDocument*> m_mapIdToScriptDocument;
    ScopedSignalConnections<> m_sigConns;
    TaskManager m_taskMgr;
    std::unordered_map<TaskId, Task> m_mapTask;
};

} // namespace Mayo
