/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_script_evaluation.h"

#include "quickjs_script_engine.h"

#include <quickjs.h>
#include <vector>

namespace Mayo {

QuickJsScriptEvaluation::QuickJsScriptEvaluation()
{
}

QuickJsScriptEvaluation::~QuickJsScriptEvaluation()
{
    std::vector<TaskId> taskIds;
    taskIds.reserve(m_pendingTasks.size());

    // Request abort for all tasks before waiting for any of them
    for (const auto& [taskId, promise] : m_pendingTasks) {
        taskIds.push_back(taskId);
        m_taskMgr.requestAbort(taskId);
    }

    // Wait for all worker functions to return
    for (const TaskId taskId : taskIds)
        m_taskMgr.waitForDone(taskId);

    // No worker can emit signalEnded after this point
    m_connTaskEnded.disconnect();

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
    QuickJsPromise promise(m_context);
    QuickJsValue promiseValue = promise.get();

    const auto taskId = m_taskMgr.newTask(std::move(job));
    m_pendingTasks.emplace(taskId, std::move(promise));
    m_taskMgr.run(taskId, TaskAutoDestroy::Off);

    return promiseValue;
}

void QuickJsScriptEvaluation::completeTask(TaskId taskId, TaskEndReason reason)
{
    auto it = m_pendingTasks.find(taskId);
    if (it == m_pendingTasks.end())
        return;

    auto promise = std::move(it->second);
    m_pendingTasks.erase(it);

    switch (reason) {
    case TaskEndReason::Completed:
        promise.resolve(JS_UNDEFINED);
        break;
    case TaskEndReason::Aborted:
        promise.reject("Task aborted");
        break;
    case TaskEndReason::Failed:
        promise.reject("Task failed");
        break;
    }

    m_taskMgr.destroy(taskId);
}

} // namespace Mayo
