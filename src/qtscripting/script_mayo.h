/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/messenger.h"
#include "../base/task_manager.h"
#include "script_global.h"
#include "script_typedefs.h"

#include <QtCore/QObject>
#include <QtQml/QJSValue>
class QJSEngine;

#include <unordered_map>

namespace Mayo {

/*! \brief Main entry point in the Mayo scripting context
 *
 *  Use global object `Mayo` in scripts:
 *  \code{.js}
 *  var doc = Mayo.mainApplication.newDocument();
 *  doc.asyncImportFile("myfile.stp");
 *  \endcode
 */
class ScriptMayo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(Ptr_ScriptApplication mainApplication READ mainApplication CONSTANT)
public:
    ScriptMayo(const ScriptEnvironment& scriptEnv, QJSEngine* jsEngine = nullptr);

    QString versionString() const;

    Ptr_ScriptApplication mainApplication() const { return m_mainApplication; }
    void setMainApplication(const ApplicationPtr& app);

    Q_INVOKABLE void traverseModelTree(
        QJSValue_ScriptDocument jsDoc, QJSValue_MayoTraverseModelTreeCallback fn
    );
    Q_INVOKABLE void traverseShape(
        QJSValue_ScriptShape jsShape, ScriptShapeType filter, QJSValue_MayoTraverseShapeCallback fn
    );

    Q_INVOKABLE bool waitForDone(int msecs = -1);
    Q_INVOKABLE bool waitForDone(TaskId taskId, int msecs = -1);
    Q_INVOKABLE bool waitForDone(QJSValueList_ArrayOfTaskId taskIdArray, int msecs = -1);

    //! \internal
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
    TaskManager& taskManager() { return m_taskMgr; }
    QJSEngine* jsEngine() const { return m_jsEngine; }
    const ScriptEnvironment& environment() const { return m_scriptEnv; }

private:
    struct Task {
        int progressPct = 0;
        QString progressStepTitle;
        TaskCallbacks callbacks;
        std::unique_ptr<MessengerBySignal> messenger;
    };
    Task* findTask(TaskId taskId);

    const ScriptEnvironment m_scriptEnv;
    QJSEngine* m_jsEngine = nullptr;
    ScriptApplication* m_mainApplication = nullptr;
    TaskManager m_taskMgr;
    std::unordered_map<TaskId, Task> m_mapTask;
};

} // namespace Mayo
