/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_script_evaluation.h"

#include "quickjs_context.h"
#include "quickjs_script_engine.h"
#include "quickjs_task.h"

#include <quickjs.h>
#include <vector>

namespace Mayo {

QuickJsScriptEvaluation::QuickJsScriptEvaluation()
{
}

QuickJsScriptEvaluation::TaskEntry* QuickJsScriptEvaluation::findTaskEntry(TaskId taskId)
{
    auto it = m_pendingTasks.find(taskId);
    return it != m_pendingTasks.end() ? &it->second : nullptr;
}

QuickJsScriptEvaluation::~QuickJsScriptEvaluation()
{
    std::vector<TaskId> taskIds;
    taskIds.reserve(m_pendingTasks.size());

    // Request abort for all tasks before waiting for any of them
    for (const auto& [taskId, taskEntry] : m_pendingTasks) {
        taskIds.push_back(taskId);
        m_taskMgr.requestAbort(taskId);
    }

    // Wait for all worker functions to return
    for (const TaskId taskId : taskIds)
        m_taskMgr.waitForDone(taskId);

    // No worker can emit signalEnded after this point
    m_connTaskEnded.disconnect();
    m_connTaskProgressChanged.disconnect();

    // No further completion jobs can be posted by these workers
    m_engine->discardJobs();

    for (const TaskId taskId : taskIds)
        m_taskMgr.destroy(taskId);

    m_pendingTasks.clear();
}

Signal<IScriptEngine::Message>& QuickJsScriptEvaluation::signalMessage() const
{
    return m_engine->signalMessage;
}

QuickJsValue QuickJsScriptEvaluation::startTask(TaskJob job)
{
    const auto taskId = m_taskMgr.newTask(std::move(job));

    TaskEntry taskEntry{m_context};
    m_pendingTasks.emplace(taskId, std::move(taskEntry));

    QuickJsValue task = QuickJsTask::create(*this, taskId);
    if (!task.isException()) {
        m_taskMgr.run(taskId, TaskAutoDestroy::Off);
    }
    else {
        m_pendingTasks.erase(taskId);
        m_taskMgr.destroy(taskId);
    }

    return task;
}

void QuickJsScriptEvaluation::completeTask(TaskId taskId, TaskEndReason reason)
{
    auto it = m_pendingTasks.find(taskId);
    if (it == m_pendingTasks.end())
        return;

    auto taskEntry = std::move(it->second);
    m_pendingTasks.erase(it);

    m_taskMgr.destroy(taskId);

    switch (reason) {
    case TaskEndReason::Completed:
        taskEntry.promise.resolve(JS_UNDEFINED);
        break;
    case TaskEndReason::Aborted:
        taskEntry.promise.reject("Task aborted");
        break;
    case TaskEndReason::Failed:
        taskEntry.promise.reject("Task failed");
        break;
    }
}

void QuickJsScriptEvaluation::notifyTaskProgress(TaskId taskId, double value)
{
    auto taskEntry = this->findTaskEntry(taskId);
    if (!taskEntry)
        return;

    // Keep a snapshot of the callbacks because a JavaScript callback may register additional
    // callbacks while the notification is being processed
    std::vector<QuickJsValue> callbacks;
    callbacks.reserve(taskEntry->progressCallbacks.size());
    for (const QuickJsValue& callback : taskEntry->progressCallbacks)
        callbacks.push_back(callback.dup());

    auto progressValue = QuickJsValue::newFloat64(m_context, value);
    if (progressValue.isException())
        return;

    for (const QuickJsValue& callback : callbacks) {
        // Invoke each callback with the progress value as its only argument
        QuickJsValue result = callback.call(JS_UNDEFINED, { progressValue.get() });

        // Release the returned value, or consume the pending exception
        if (result.isException()) {
            QuickJsValue exception = QuickJsContext::takeException(m_context);
            IScriptEngine::Message errMsg;
            errMsg.type = MessageType::Error;
            errMsg.text = exception.getProperty("message").toStdString().value_or(std::string{});
            // TODO Assign IScriptEngine::Message::contextFile
            this->signalMessage().send(errMsg);
            continue; // Continue notifying the remaining callbacks
        }
    }
}

} // namespace Mayo
