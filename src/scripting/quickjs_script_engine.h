/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"
#include "quickjs_value.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>
#include <vector>
#include <thread>

namespace Mayo {

class QuickJsScriptEvaluation;

// Provides script engine based on QuickJS
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
    friend class QuickJsScriptEvaluation;

    using Job = std::function<void()>;
    void postJob(Job job);
    void processJobs();
    void discardJobs();

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
    bool m_isEvaluateShuttingDown{false};
    std::atomic<bool> m_stopRequested{false};

    std::vector<EvaluateCallback> m_preEvaluateCallbacks;
};

} // namespace Mayo
