/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_script_engine.h"

#include "quickjs_context.h"
#include "quickjs_promise.h"
#include "quickjs_value.h"
#include "../base/task_manager.h"

#include <gsl/util>
#include <fmt/format.h>
#include <quickjs.h>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace Mayo {

namespace {

std::string readAll(std::istream& inputStream)
{
    return std::string{
        std::istreambuf_iterator<char>(inputStream),
        std::istreambuf_iterator<char>()
    };
}

std::string getJsValueString(JSContext* context, JSValueConst value, std::string_view strDefaultIfNull = {})
{
    const char* cstr = JS_ToCString(context, value);
    if (cstr) {
        const std::string str = cstr;
        JS_FreeCString(context, cstr);
        return str;
    }

    return std::string{strDefaultIfNull};
}

std::any jsValueToAny(JSContext* ctx, JSValueConst value)
{
    if (JS_IsUndefined(value) || JS_IsNull(value))
        return {};

    if (JS_IsBool(value))
        return bool(JS_ToBool(ctx, value));

    if (JS_IsNumber(value)) {
        double number = 0.;
        if (JS_ToFloat64(ctx, &number, value) == 0)
            return number;

        return {};
    }

    if (JS_IsString(value))
        return getJsValueString(ctx, value);

    // Objects, arrays, functions, etc. are deliberately not exposed through std::any yet
    // A dedicated ScriptValue can be introduced later
    return {};
}

std::any jsValueToAny(const QuickJsValue& jsValue)
{
    return jsValueToAny(jsValue.context(), jsValue.get());
}

std::string currentScriptFile(JSContext* context)
{
    // Skip the native function implementing console.log() and get the JavaScript caller's
    // script/module filename
    const JSAtom atom = JS_GetScriptOrModuleName(context, 1);
    if (atom == JS_ATOM_NULL)
        return {};

    const char* cstr = JS_AtomToCString(context, atom);
    const std::string str = cstr ? cstr : std::string{};
    JS_FreeCString(context, cstr);

    JS_FreeAtom(context, atom);
    return str;
}

JSValue jsConsoleWrite(JSContext* context, int argc, JSValueConst* argv, MessageType type)
{
    auto evalState = static_cast<QuickJsScriptEvaluation*>(JS_GetContextOpaque(context));
    if (!evalState)
        return JS_UNDEFINED;

    std::string text;
    for (int i = 0; i < argc; ++i) {
        if (i > 0)
            text += ' ';

        text += getJsValueString(context, argv[i], "<unprintable>");
    }

    IScriptEngine::Message message;
    message.type = type;
    message.text = std::move(text);
    message.contextFile = currentScriptFile(context);
    evalState->signalMessage().send(message);
    return JS_UNDEFINED;
}

JSValue jsConsoleLog(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    return jsConsoleWrite(context, argc, argv, MessageType::Trace);
}

JSValue jsConsoleInfo(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    return jsConsoleWrite(context, argc, argv, MessageType::Info);
}

JSValue jsConsoleWarn(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    return jsConsoleWrite(context, argc, argv, MessageType::Warning);
}

JSValue jsConsoleError(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    return jsConsoleWrite(context, argc, argv, MessageType::Error);
}

void installConsole(JSContext* context)
{
    JSValue console = JS_NewObject(context);
    JS_SetPropertyStr(context, console, "log", JS_NewCFunction(context, &jsConsoleLog, "log", -1));
    JS_SetPropertyStr(context, console, "info", JS_NewCFunction(context, &jsConsoleInfo, "info", -1));
    JS_SetPropertyStr(context, console, "warn", JS_NewCFunction(context, &jsConsoleWarn, "warn", -1));
    JS_SetPropertyStr(context, console, "error", JS_NewCFunction(context, &jsConsoleError, "error", -1));

    JSValue global = JS_GetGlobalObject(context);
    JS_SetPropertyStr(context, global, "console", console);
    JS_FreeValue(context, global);
}

// QuickJS callback of type JSModuleNormalizeFunc
// Used with JS_SetModuleLoaderFunc()
char* moduleNormalize(JSContext* context, const char* moduleBaseName, const char* moduleName, void*)
{
    const auto basePath = std::filesystem::u8path(moduleBaseName);
    const auto modulePath =
        filepathAbsolute(basePath.parent_path() / std::filesystem::u8path(moduleName))
        .lexically_normal();

    const std::string path = modulePath.u8string();
    char* result = static_cast<char*>(js_malloc(context, path.size() + 1));
    if (!result)
        return nullptr;

    std::memcpy(result, path.c_str(), path.size() + 1);
    return result;
}

// QuickJS callback of type JSModuleLoaderFunc
// Used with JS_SetModuleLoaderFunc()
JSModuleDef* moduleLoader(JSContext* context, const char* moduleName, void*)
{
    std::ifstream file(std::filesystem::u8path(moduleName), std::ios::binary);
    if (!file) {
        JS_ThrowReferenceError(context, "Failed to load module '%s'", moduleName);
        return nullptr;
    }

    const std::string source = readAll(file);
    const auto jsEvalFlags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
    JSValue value = JS_Eval(context, source.c_str(), source.size(), moduleName, jsEvalFlags);
    if (JS_IsException(value))
        return nullptr;

    auto module = static_cast<JSModuleDef*>(JS_VALUE_GET_PTR(value));
    JS_FreeValue(context, value);
    return module;
}

// QuickJS callback of type JSInterruptHandler
// Used with JS_SetInterruptHandler()
int interruptHandler(JSRuntime*, void* opaque)
{
    auto engine = static_cast<QuickJsScriptEngine*>(opaque);
    return engine->isStopRequested() ? 1 : 0;
}

} // namespace

struct QuickJsScriptEvaluation::Private {
    using PendingTasks = std::unordered_map<TaskId, QuickJsPromise>;

    QuickJsScriptEngine* engine{nullptr};
    JSContext* context{nullptr};

    TaskManager taskMgr;
    ScopedSignalConnection connTaskEnded;
    PendingTasks pendingTasks;

    // Completes a pending task and settles its associated JS Promise
    void completeTask(TaskId taskId, TaskEndReason reason);
};

QuickJsScriptEvaluation::QuickJsScriptEvaluation()
    : d(new QuickJsScriptEvaluation::Private)
{
}

QuickJsScriptEvaluation::~QuickJsScriptEvaluation()
{
    delete d;
}

Signal<IScriptEngine::Message>& QuickJsScriptEvaluation::signalMessage() const
{
    return d->engine->signalMessage;
}

QuickJsValue QuickJsScriptEvaluation::startTask(TaskJob job)
{
    QuickJsPromise promise(d->context);
    QuickJsValue promiseValue = promise.get();

    const auto taskId = d->taskMgr.newTask(std::move(job));
    d->pendingTasks.emplace(taskId, std::move(promise));
    d->taskMgr.run(taskId, TaskAutoDestroy::Off);

    return promiseValue;
}

void QuickJsScriptEvaluation::Private::completeTask(TaskId taskId, TaskEndReason reason)
{
    auto it = this->pendingTasks.find(taskId);
    if (it == this->pendingTasks.end())
        return;

    auto promise = std::move(it->second);
    this->pendingTasks.erase(it);

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

    this->taskMgr.destroy(taskId);
}

QuickJsScriptEngine::~QuickJsScriptEngine()
{
    QuickJsScriptEngine::stopEvaluate();
    if (m_evaluateThread.joinable())
        m_evaluateThread.join();
}

void QuickJsScriptEngine::startEvaluate()
{
    std::thread previousThread;

    {
        std::lock_guard lock(m_mutex);

        if (m_isEvaluateRunning)
            return;

        previousThread = std::move(m_evaluateThread);

        const std::string script = this->script();
        const FilePath filePath = this->scriptFilePath();

        m_stopRequested.store(false);
        m_isEvaluateRunning = true;
        m_evaluateThread = std::thread([=]{this->evaluateWorker(script, filePath); });
    }

    if (previousThread.joinable())
        previousThread.join();
}

void QuickJsScriptEngine::stopEvaluate()
{
    m_stopRequested.store(true);
    m_jobCondition.notify_all();
}

bool QuickJsScriptEngine::isEvaluateRunning() const
{
    [[maybe_unused]] std::lock_guard lock(m_mutex);
    return m_isEvaluateRunning;
}

bool QuickJsScriptEngine::waitForEvaluateEnd(int msecs)
{
    std::unique_lock lock(m_mutex);
    const auto predicate = [=]{ return !m_isEvaluateRunning; };

    if (msecs < 0) {
        m_evaluateEndCondition.wait(lock, predicate);
        return true;
    }

    return m_evaluateEndCondition.wait_for(lock, std::chrono::milliseconds(msecs), predicate);
}

bool QuickJsScriptEngine::isStopRequested() const
{
    return m_stopRequested.load();
}

void QuickJsScriptEngine::addPreEvaluateCallback(EvaluateCallback callback)
{
    if (callback)
        m_preEvaluateCallbacks.push_back(std::move(callback));
}

void QuickJsScriptEngine::postJob(Job job)
{
    {
        std::lock_guard lock(m_mutex);

        if (!m_isEvaluateRunning || m_stopRequested)
            return;

        m_jobQueue.push(std::move(job));
    }

    m_jobCondition.notify_one();
}

void QuickJsScriptEngine::processJobs()
{
    std::queue<Job> jobs;
    {
        std::lock_guard lock(m_mutex);
        jobs.swap(m_jobQueue);
    }

    while (!jobs.empty()) {
        Job job = std::move(jobs.front());
        jobs.pop();
        job();
    }
}

void QuickJsScriptEngine::evaluateWorker(const std::string& script, const FilePath& scriptFilePath)
{
    this->signalEvaluateStarted.send();

    // Variables used for exit status/result
    Result result;
    EndReason endReason = EndReason::Finished;

    // Final action executed when function exits
    auto onExit = gsl::finally([&]{ this->finalizeEvaluation(result, endReason); });

    // Helper function to emit error message
    auto handleError = [&](const QuickJsValue& jsError, std::string_view messageIfNoJsError = {}) {
        result.success = false;
        this->sendErrorMessage(scriptFilePath, jsError, messageIfNoJsError);
    };

    // Helper function to acknowledge stop request
    auto handleStopRequest = [&](JSContext* jsContext) {
        QuickJsContext::takeException(jsContext);
        endReason = EndReason::Stopped;
    };

    // Create JS runtime
    QuickJsRuntime runtime{JS_NewRuntime()};
    if (!runtime)
        return handleError({}, "Failed to create QuickJS runtime");

    JS_SetInterruptHandler(runtime.get(), &interruptHandler, this);
    JS_SetModuleLoaderFunc(runtime.get(), &moduleNormalize, &moduleLoader, this);

    // Create JS context
    QuickJsContext context{JS_NewContext(runtime.get())};
    if (!context)
        return handleError({}, "Failed to create QuickJS context");

    QuickJsScriptEvaluation evalState;
    this->initEvaluationState(evalState, context.get());
    auto gc = gsl::finally([&]{ JS_RunGC(runtime.get()); });

    JS_SetContextOpaque(context.get(), &evalState);
    installConsole(context.get());

    for (const EvaluateCallback& callback : m_preEvaluateCallbacks)
        callback(context.get(), evalState);

    // Evaluate JS program
    auto moduleValue = context.eval(script, scriptFilePath, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (moduleValue.isException())
        return handleError(context.takeException());

    // Keep the module definition pointer for retrieving its namespace after evaluation.
    // The JSValue reference is consumed by evalFunction(), but the module definition remains owned
    // by the QuickJS runtime
    auto mainModule = static_cast<JSModuleDef*>(moduleValue.ptr());

    auto value = context.evalFunction(std::move(moduleValue)); // Consumed by evalFunction()

    if (value.isException()) {
        if (this->isStopRequested())
            return handleStopRequest(context.get());
        else
            return handleError(context.takeException());
    }

    // Execute pending JavaScript jobs until the module evaluation promise is settled
    auto pendingJobsResult = this->execPendingJobs(value, runtime.get());
    switch (pendingJobsResult.type) {
    case PendingJobsResult::Type::Error:
        return handleError(std::move(pendingJobsResult.error));
    case PendingJobsResult::Type::StopRequest:
        return handleStopRequest(pendingJobsResult.stopRequestContext);
    }

    // Retrieve the default export from the module namespace as the script result
    auto defaultValue = context.getModuleDefaultExport(mainModule);
    if (defaultValue.isException())
        return handleError(defaultValue);

    // Success
    result.success = true;    
    result.value = jsValueToAny(defaultValue);
}

void QuickJsScriptEngine::initEvaluationState(EvaluationState& state, JSContext *context)
{
    state.d->engine = this;
    state.d->context = context;
    state.d->connTaskEnded = state.d->taskMgr.signalEnded.connectSlot(
        [&](TaskId taskId, TaskEndReason reason) {
            this->postJob([&state, taskId, reason] {
                state.d->completeTask(taskId, reason);
            });
        }
    );
}

void QuickJsScriptEngine::sendErrorMessage(
        const FilePath& contextFile, const QuickJsValue& jsError, std::string_view messageIfNoJsError
    )
{
    Message message;
    message.type = MessageType::Error;
    message.text =
        jsError
            ? jsError.toStdString().value_or("Unknown JavaScript exception")
            : std::string{messageIfNoJsError};
    message.contextFile = !contextFile.empty() ? contextFile.u8string() : std::string{"<script>"};
    this->signalMessage.send(message);
}

QuickJsScriptEngine::PendingJobsResult
QuickJsScriptEngine::execPendingJobs(const QuickJsValue& promise, JSRuntime* runtime)
{
    auto stopRequestResult = [](JSContext* context) {
        PendingJobsResult result;
        result.type = PendingJobsResult::Type::StopRequest;
        result.stopRequestContext = context;
        return result;
    };

    auto errorResult = [](QuickJsValue error) {
        PendingJobsResult result;
        result.type = PendingJobsResult::Type::Error;
        result.error = std::move(error);
        return result;
    };

    while (promise.promiseState() == JS_PROMISE_PENDING) {
        // Process jobs posted from outside the QuickJS thread
        this->processJobs();

        if (this->isStopRequested())
            return stopRequestResult(promise.context());

        // Execute one pending QuickJS job
        JSContext* jobContext = nullptr;
        const int jobResult = JS_ExecutePendingJob(runtime, &jobContext);

        // A JavaScript exception occurred while executing the job
        if (jobResult < 0) {
            if (this->isStopRequested())
                return stopRequestResult(jobContext);

            return errorResult(QuickJsContext::takeException(jobContext));
        }

        // No QuickJS job is currently pending
        if (jobResult == 0) {
            if (this->isStopRequested())
                return stopRequestResult(promise.context());

            std::unique_lock lock(m_mutex);
            m_jobCondition.wait(lock, [this]{ return !m_jobQueue.empty() || this->isStopRequested(); });
            continue;
        }

        // A QuickJS job was executed
        if (this->isStopRequested())
            return stopRequestResult(jobContext);
    }

    // Check the final state of the module evaluation promise
    const JSPromiseStateEnum promiseState = promise.promiseState();

    if (promiseState == JS_PROMISE_REJECTED) {
        if (this->isStopRequested())
            return stopRequestResult(promise.context());
        else
            return errorResult(promise.promiseResult());
    }
    else if (promiseState != JS_PROMISE_FULFILLED) {
        return errorResult(QuickJsValue::newError(promise.context(), "Invalid module evaluation state"));
    }

    return {};
}

void QuickJsScriptEngine::finalizeEvaluation(const Result& result, EndReason endReason)
{
    this->signalEvaluateEnded.send(result, endReason);

    {
        std::lock_guard lock(m_mutex);
        m_isEvaluateRunning = false;
    }

    m_evaluateEndCondition.notify_all();
}

} // namespace Mayo
