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

namespace {

QuickJsValue toQuickJsValue(JSContext* context, const std::any& value)
{
    if (!value.has_value())
        return { context, JS_UNDEFINED };

    if (const auto* boolValue = std::any_cast<bool>(&value))
        return QuickJsValue::newBool(context, *boolValue);

    if (const auto* intValue = std::any_cast<int>(&value))
        return QuickJsValue::newInt32(context, *intValue);

    if (const auto* doubleValue = std::any_cast<double>(&value))
        return QuickJsValue::newFloat64(context, *doubleValue);

    if (const auto* stringValue = std::any_cast<std::string>(&value))
        return QuickJsValue::newString(context, *stringValue);

    return QuickJsContext::throwInternalError(context, "Unsupported std::any type");
}

} // namespace

QuickJsScriptEvaluation::QuickJsScriptEvaluation()
{
}

QuickJsScriptEvaluation::TaskEntry* QuickJsScriptEvaluation::findTaskEntry(TaskId taskId)
{
    auto it = m_pendingTasks.find(taskId);
    return it != m_pendingTasks.end() ? it->second.get() : nullptr;
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
    m_connTaskProgressStep.disconnect();

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

QuickJsValue QuickJsScriptEvaluation::startTask(TaskFunction fn, TaskResultConverter resultConv)
{
    auto taskEntry = std::make_unique<TaskEntry>(m_context, std::move(resultConv));
    TaskEntry* const taskEntryPtr = taskEntry.get();

    const TaskId taskId = m_taskMgr.newTask([=](TaskProgress* progress) {
        taskEntryPtr->result = fn(progress);
    });
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

QuickJsValue QuickJsScriptEvaluation::getTaskJsResult(const TaskEntry& taskEntry) const
{
    try {
        return taskEntry.resultConverter
            ? taskEntry.resultConverter(m_context, taskEntry.result)
            : toQuickJsValue(m_context, taskEntry.result)
        ;
    }
    catch (const std::exception& err) {
        return QuickJsContext::throwInternalError(m_context, err.what());
    }
    catch (...) {
        return QuickJsContext::throwInternalError(m_context, "Task result conversion failed");
    }
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
    case TaskEndReason::Completed: {
        const QuickJsValue jsResult = this->getTaskJsResult(*taskEntry);
        if (jsResult.isException())
            taskEntry->promise.reject(QuickJsContext::takeException(m_context).get());
        else
            taskEntry->promise.resolve(jsResult.get());
    }
        break;
    case TaskEndReason::Aborted:
        taskEntry->promise.reject("Task aborted");
        break;
    case TaskEndReason::Failed:
        taskEntry->promise.reject("Task failed");
        break;
    }
}

void QuickJsScriptEvaluation::notifyTaskProgressEvent(TaskId taskId, double value)
{
    auto taskEntry = this->findTaskEntry(taskId);
    if (!taskEntry || taskEntry->progressEventCallbacks.empty())
        return;

    std::vector<QuickJsValue> callbacks;
    callbacks.reserve(taskEntry->progressEventCallbacks.size());
    for (const QuickJsValue& callback : taskEntry->progressEventCallbacks)
        callbacks.push_back(callback.dup());

    auto progress = QuickJsValue::newObject(m_context);
    if (progress.isException()) {
        QuickJsContext::takeException(m_context);
        return;
    }

    const std::string& message = taskEntry->progressMessage;
    if (!(progress.setProperty("value", QuickJsValue::newFloat64(m_context, value))
          && progress.setProperty("message", QuickJsValue::newString(m_context, message))
        ))
    {
        QuickJsContext::takeException(m_context);
        return;
    }

    for (const QuickJsValue& callback : callbacks) {
        QuickJsValue result = callback.call(JS_UNDEFINED, { progress.get() });
        if (result.isException()) {
            QuickJsValue exception = QuickJsContext::takeException(m_context);
            IScriptEngine::Message errMsg;
            errMsg.type = MessageType::Error;
            errMsg.text = exception.getProperty("message").toStdString().value_or(std::string{});
            this->signalMessage().send(errMsg);
        }
    }
}

void QuickJsScriptEvaluation::updateTaskProgressMessage(TaskId taskId, std::string message)
{
    auto taskEntry = this->findTaskEntry(taskId);
    if (taskEntry) {
        taskEntry->progressMessage = std::move(message);
        this->notifyTaskProgressEvent(taskId, m_taskMgr.progress(taskId));
    }
}

} // namespace Mayo
