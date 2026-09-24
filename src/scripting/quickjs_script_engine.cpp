/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_script_engine.h"

#include <quickjs.h>

#include <gsl/util>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Mayo {

namespace {

std::string readTextFile(const FilePath& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to open script file");

    std::ostringstream stream;
    stream << file.rdbuf();
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

} // namespace


QuickJsScriptEngine::~QuickJsScriptEngine()
{
    this->stopEvaluate();
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

    m_stopRequested.store(false);
    m_evaluateThread = std::thread([=]{ this->evaluateWorker(filePath); });
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

int QuickJsScriptEngine::interruptHandler(JSRuntime*, void* opaque)
{
    auto engine = static_cast<QuickJsScriptEngine*>(opaque);
    return engine->m_stopRequested.load() ? 1 : 0;
}

void QuickJsScriptEngine::evaluateWorker(const FilePath& scriptFilePath)
{
    this->signalEvaluateStarted.send();

    Result result;
    EndReason endReason = EndReason::Finished;
    JSRuntime* runtime = nullptr;
    JSContext* context = nullptr;
    JSValue value = JS_UNDEFINED;

    auto error = [&](std::string_view strMessage) {
        result.success = false;
        this->emitMessage(MessageType::Error, strMessage, scriptFilePath.u8string());
    };

    [[maybe_unused]] auto onExit = gsl::finally([&]{
        if (context) {
            JS_FreeValue(context, value);
            JS_FreeContext(context);
        }
        if (runtime)
            JS_FreeRuntime(runtime);
        m_isEvaluateRunning.store(false);
        m_evaluateEndCondition.notify_all();
        this->signalEvaluateEnded.send(result, endReason);
    });

    runtime = JS_NewRuntime();
    if (!runtime)
        return error("Failed to create QuickJS runtime");

    JS_SetInterruptHandler(runtime, &QuickJsScriptEngine::interruptHandler, this);

    context = JS_NewContext(runtime);
    if (!context)
        return error("Failed to create QuickJS context");

    const std::string source = readTextFile(scriptFilePath);
    value = JS_Eval(
        context, source.c_str(), source.size(), scriptFilePath.u8string().c_str(), JS_EVAL_TYPE_GLOBAL
    );

    if (m_stopRequested.load()) {
        endReason = EndReason::Stopped;
        return;
    }

    if (JS_IsException(value)) {
        JSValue exception = JS_GetException(context);
        const char* ctext = JS_ToCString(context, exception);
        const std::string strText = ctext ? ctext : "JavaScript unknown exception";
        JS_FreeCString(context, ctext);
        JS_FreeValue(context, exception);
        return error(strText);
    }

    result.success = true;
    result.value = jsValueToAny(context, value);
}

void QuickJsScriptEngine::emitMessage(
        MessageType type, std::string_view text, std::string_view contextFile, int contextLine
    )
{
    Message message;
    message.type = type;
    message.text = text;
    message.contextFile = contextFile;
    message.contextLine = contextLine;

    this->signalMessage.send(message);
}

} // namespace Mayo
