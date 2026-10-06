/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"
#include "quickjs_value.h"
#include "../base/task_job.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>
#include <vector>
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

    Signal<IScriptEngine::Message>& signalMessage() const;

    // Starts an asynchronous task and returns a JS Promise that is settled when the task completes
    QuickJsValue startTask(TaskJob job);

private:
    QuickJsScriptEvaluation();

    friend class QuickJsScriptEngine;
    struct Private;
    Private* const d{nullptr};
};

class QuickJsScriptEngine : public IScriptEngine {
public:
    using EvaluationState = QuickJsScriptEvaluation;
    using EvaluateCallback = std::function<void(JSContext*, EvaluationState&)>;

    ~QuickJsScriptEngine();

    void startEvaluate() override;
    void stopEvaluate() override;
    bool isEvaluateRunning() const override;
    bool waitForEvaluateEnd(int msecs = -1) override;

    bool isStopRequested() const;

    void addPreEvaluateCallback(EvaluateCallback callback);

private:
    using Job = std::function<void()>;
    void postJob(Job job);
    void processJobs();

    void evaluateWorker(const std::string& script, const FilePath& scriptFilePath);

    void initEvaluationState(EvaluationState& state, JSContext* context);

    // Result returned by execPendingJobs() function
    struct PendingJobsResult {
        enum class Type {
            Success, StopRequest, Error
        };
        Type type{Type::Success};
        JSContext* stopRequestContext{nullptr}; // If type==StopRequest
        QuickJsValue error; // If type==Error
    };

    // Execute pending JavaScript jobs until the module evaluation promise is settled
    PendingJobsResult execPendingJobs(const QuickJsValue& promise, JSRuntime* runtime);

    void sendErrorMessage(
        const FilePath& contextFile, const QuickJsValue& jsError, std::string_view messageIfNoJsError
    );

    void finalizeEvaluation(const Result& result, EndReason endReason);

    // This mutex protects state and state transitions
    // Long-running or reentrant operations are performed outside the lock
    mutable std::mutex m_mutex;

    std::queue<Job> m_jobQueue;
    std::condition_variable m_jobCondition;

    std::condition_variable m_evaluateEndCondition;
    std::thread m_evaluateThread;
    bool m_isEvaluateRunning{false};
    std::atomic<bool> m_stopRequested{false};

    std::vector<EvaluateCallback> m_preEvaluateCallbacks;
};

} // namespace Mayo
