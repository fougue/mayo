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

    if (JS_IsString(value)) {
        const char* str = JS_ToCString(ctx, value);
        if (!str)
            return {};

        std::string result(str);
        JS_FreeCString(ctx, str);
        return result;
    }

    // Objects, arrays, functions, etc. are deliberately not exposed through std::any yet
    // A dedicated ScriptValue can be introduced later
    return {};
}

std::string toStdString(JSContext* context, JSValueConst value, std::string_view strDefaultIfNull)
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
    const std::string strText = toStdString(context, exception, "JavaScript unknown exception");
    JS_FreeValue(context, exception);
    return strText;
}

std::string currentScriptFile(JSContext* context)
{
    const JSAtom atom = JS_GetScriptOrModuleName(context, 0);
    if (atom == JS_ATOM_NULL)
        return {};

    const char* cstr = JS_AtomToCString(context, atom);
    const std::string str = cstr ? cstr : "";
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

        text += toStdString(context, argv[i], "<unprintable>");
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
    std::ifstream file(moduleName, std::ios::binary);
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
    bool expected = false;
    if (!m_isEvaluateRunning.compare_exchange_strong(expected, true))
        return; // Already running

    // The previous worker may have finished but its std::thread may still be joinable. It must be
    // joined before assigning a new thread
    if (m_evaluateThread.joinable())
        m_evaluateThread.join();

    const FilePath filePath = this->scriptFilePath();
    const std::string script = this->script();

    m_stopRequested.store(false);
    m_evaluateThread = std::thread([=]{ this->evaluateWorker(script, filePath); });
}

void QuickJsScriptEngine::stopEvaluate()
{
    m_stopRequested.store(true);
}

bool QuickJsScriptEngine::isEvaluateRunning() const
{
    return m_isEvaluateRunning.load();
}

bool QuickJsScriptEngine::waitForEvaluateEnd(int msecs)
{
    if (!m_isEvaluateRunning.load())
        return true;

    std::unique_lock lock(m_evaluateEndMutex);

    const auto predicate = [=]{ return !m_isEvaluateRunning.load(); };

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
        m_isEvaluateRunning.store(false);
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

    // Create JS runtine
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

    value = JS_EvalFunction(context, moduleValue);
    moduleValue = JS_UNDEFINED; // Consumed by JS_EvalFunction()

    // Handle stop request (if any)
    if (this->isStopRequested()) {
        endReason = EndReason::Stopped;
        return;
    }

    // Handle exception that eventually occured during script execution
    if (JS_IsException(value))
        return error(getJsExceptionString(context, value));

    // Success
    result.success = true;
    result.value = jsValueToAny(context, value);
}

} // namespace Mayo
