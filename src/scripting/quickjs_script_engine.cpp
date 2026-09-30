/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_script_engine.h"

#include <quickjs.h>

#include <gsl/util>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <string>

namespace Mayo {

namespace {

std::string readAll(std::istream& inputStream)
{
    std::ostringstream stream;
    stream << inputStream.rdbuf();
    return stream.str();
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

std::string getJsExceptionString(JSContext* context, JSValueConst value)
{
    if (!JS_IsException(value))
        return {};

    JSValue exception = JS_GetException(context);
    const std::string strText = getJsValueString(context, exception, "Unknown JavaScript exception");
    JS_FreeValue(context, exception);
    return strText;
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
    auto engine = static_cast<QuickJsScriptEngine*>(JS_GetContextOpaque(context));
    if (!engine)
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
    engine->signalMessage.send(message);
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

void QuickJsScriptEngine::evaluateWorker(const std::string& script, const FilePath& scriptFilePath)
{
    this->signalEvaluateStarted.send();

    const auto strScriptFilePath = scriptFilePath.u8string();

    // Variables used for exit status/result
    Result result;
    EndReason endReason = EndReason::Finished;
    // Variables used for script execution
    JSRuntime* runtime = nullptr;
    JSContext* context = nullptr;
    JSValue moduleValue = JS_UNDEFINED;
    JSValue value = JS_UNDEFINED;
    JSModuleDef* mainModule = nullptr;

    // Final action executed when function exits
    [[maybe_unused]] auto onExit = gsl::finally([&]{
        if (context) {
            JS_FreeValue(context, value);
            JS_FreeValue(context, moduleValue);
            JS_FreeContext(context);
        }

        if (runtime) {
            JS_FreeRuntime(runtime);
        }

        {
            std::lock_guard lock(m_mutex);
            m_isEvaluateRunning = false;
        }
        m_evaluateEndCondition.notify_all();
        this->signalEvaluateEnded.send(result, endReason);
    });

    // Helper function to emit error message
    auto error = [&](std::string_view strMessage) {
        result.success = false;
        Message message;
        message.type = MessageType::Error;
        message.text = strMessage;
        message.contextFile = !strScriptFilePath.empty() ? strScriptFilePath : std::string{"<script>"};
        this->signalMessage.send(message);
    };

    auto acknowledgeStop = [&]{ endReason = EndReason::Stopped; };

    // Create JS runtime
    runtime = JS_NewRuntime();
    if (!runtime)
        return error("Failed to create QuickJS runtime");

    JS_SetInterruptHandler(runtime, &interruptHandler, this);
    JS_SetModuleLoaderFunc(runtime, &moduleNormalize, &moduleLoader, this);

    // Create JS context
    context = JS_NewContext(runtime);
    if (!context)
        return error("Failed to create QuickJS context");

    JS_SetContextOpaque(context, this);
    installConsole(context);

    // Evaluate JS program
    const auto jsEvalFlags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
    moduleValue = JS_Eval(context, script.c_str(), script.size(), strScriptFilePath.c_str(), jsEvalFlags);
    if (JS_IsException(moduleValue))
        return error(getJsExceptionString(context, moduleValue));

    mainModule = static_cast<JSModuleDef*>(JS_VALUE_GET_PTR(moduleValue));

    value = JS_EvalFunction(context, moduleValue);
    moduleValue = JS_UNDEFINED; // Consumed by JS_EvalFunction()

    if (JS_IsException(value)) {
        // The exception may have been caused by an interruption requested by the caller
        if (this->isStopRequested())
            return acknowledgeStop();

        return error(getJsExceptionString(context, value));
    }

    // Execute pending JavaScript jobs until the module evaluation promise is settled
    while (JS_PromiseState(context, value) == JS_PROMISE_PENDING) {
        JSContext* jobContext = nullptr;
        const int jobResult = JS_ExecutePendingJob(runtime, &jobContext);

        // A JavaScript exception occurred while executing the job
        if (jobResult < 0) {
            // The exception may have been caused by an interruption requested by the caller
            if (this->isStopRequested())
                return acknowledgeStop();

            JSValue exception = JS_GetException(jobContext);
            const std::string message = getJsValueString(jobContext, exception, "Unknown JavaScript exception");
            JS_FreeValue(jobContext, exception);
            return error(message);
        }

        // No job is pending while the module evaluation promise is still pending
        if (jobResult == 0)
            return error("Module evaluation did not complete");

        // Stop the evaluation if requested by the caller
        if (this->isStopRequested())
            return acknowledgeStop();
    }

    // Check the final state of the module evaluation promise
    const JSPromiseStateEnum promiseState = JS_PromiseState(context, value);

    if (promiseState == JS_PROMISE_REJECTED) {
        JSValue exception = JS_PromiseResult(context, value);
        if (this->isStopRequested()) {
            JS_FreeValue(context, exception);
            return acknowledgeStop();
        }

        const std::string message = getJsValueString(context, exception, "Unknown JavaScript exception");
        JS_FreeValue(context, exception);
        return error(message);
    }
    else if (promiseState != JS_PROMISE_FULFILLED) {
        return error("Invalid module evaluation state");
    }

    // Success
    result.success = true;

    if (mainModule) {
        // Retrieve the default export from the module namespace as the script result
        JSValue namespaceValue = JS_GetModuleNamespace(context, mainModule);
        if (!JS_IsException(namespaceValue)) {
            JSValue defaultValue = JS_GetPropertyStr(context, namespaceValue, "default");
            if (!JS_IsException(defaultValue))
                result.value = jsValueToAny(context, defaultValue);

            JS_FreeValue(context, defaultValue);
        }

        JS_FreeValue(context, namespaceValue);
    }
}

} // namespace Mayo
