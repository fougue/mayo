/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"
#include "quickjs_promise.h"
#include "quickjs_value.h"

#include "../base/task_manager.h"

#include <any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

struct JSContext;

namespace Mayo {

class QuickJsScriptEngine;

// Represents the state and resources associated with a script evaluation
//
// QuickJsScriptEvaluation is created for the duration of a script evaluation (with
// QuickJsScriptEngine::startEvaluate()) and provides the context required to start and track
// asynchronous tasks associated with the evaluation
// The evaluation must remain alive until all tasks started through this object have completed
class QuickJsScriptEvaluation {
public:
    ~QuickJsScriptEvaluation();

    // Not copyable
    QuickJsScriptEvaluation(const QuickJsScriptEvaluation&) = delete;
    QuickJsScriptEvaluation& operator=(const QuickJsScriptEvaluation&) = delete;
    QuickJsScriptEvaluation(QuickJsScriptEvaluation&&) = delete;
    QuickJsScriptEvaluation& operator=(QuickJsScriptEvaluation&&) = delete;

    JSContext* context() const { return m_context; }

    Signal<IScriptEngine::Message>& signalMessage() const;

    using ScriptTaskJob = std::function<std::any(TaskProgress*)>;
    using TaskResultConverter = std::function<QuickJsValue(JSContext*, const std::any&)>;

    // Starts an asynchronous task and returns a JS Promise that is settled when the task completes
    QuickJsValue startTask(ScriptTaskJob job, TaskResultConverter resultConv = {});

private:
    friend class QuickJsScriptEngine;
    friend class QuickJsTask;

    struct TaskEntry {
        TaskEntry(JSContext* context, TaskResultConverter converter)
            : promise(context),
              resultConverter(std::move(converter))
        {}

        QuickJsPromise promise;
        TaskResultConverter resultConverter;
        std::any result;
        std::vector<QuickJsValue> progressCallbacks;
    };

    using PendingTasks = std::unordered_map<TaskId, std::unique_ptr<TaskEntry>>;

    // Constructible by QuickJsScriptEngine only
    QuickJsScriptEvaluation();

    TaskEntry* findTaskEntry(TaskId taskId);

    // Completes a pending task and settles its associated JS Promise
    void completeTask(TaskId taskId, TaskEndReason reason);

    void notifyTaskProgress(TaskId taskId, double value);

    QuickJsValue getTaskJsResult(const TaskEntry& taskEntry) const;

    QuickJsScriptEngine* m_engine{nullptr};
    JSContext* m_context{nullptr};

    TaskManager m_taskMgr;
    SignalConnectionHandle m_connTaskEnded;
    SignalConnectionHandle m_connTaskProgressChanged;
    PendingTasks m_pendingTasks;
};

} // namespace Mayo
