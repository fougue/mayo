/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"
#include "quickjs_value.h"
#include "../base/task_manager.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>
#include <thread>

namespace Mayo {

// Represents the state and resources associated with a script evaluation
//
// QuickJsScriptEvaluation is created for the duration of a script evaluation (with
// QuickJsScriptEngine::startEvaluate())and provides the context required to start and track
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

    // Starts an asynchronous task and returns a JS Promise that is settled when the task completes
    QuickJsValue startTask(TaskJob job);

    // Completes a pending task and settles its associated JS Promise
    void completeTask(TaskId taskId, TaskEndReason reason);

private:
    QuickJsScriptEvaluation();

    friend class QuickJsScriptEngine;
    struct Private;
    Private* const d{nullptr};
};

class QuickJsScriptEngine : public IScriptEngine {
public:
    using Evaluation = QuickJsScriptEvaluation;

    ~QuickJsScriptEngine();

    void startEvaluate() override;
    void stopEvaluate() override;
    bool isEvaluateRunning() const override;
    bool waitForEvaluateEnd(int msecs = -1) override;

    bool isStopRequested() const;

private:
    using Job = std::function<void()>;
    void postJob(Job job);
    void processJobs();

    void evaluateWorker(const std::string& script, const FilePath& scriptFilePath);

    // This mutex protects state and state transitions
    // Long-running or reentrant operations are performed outside the lock
    mutable std::mutex m_mutex;

    std::queue<Job> m_jobQueue;
    std::condition_variable m_jobCondition;

    std::condition_variable m_evaluateEndCondition;
    std::thread m_evaluateThread;
    bool m_isEvaluateRunning{false};
    std::atomic<bool> m_stopRequested{false};

    TaskManager m_taskMgr;
};

} // namespace Mayo
