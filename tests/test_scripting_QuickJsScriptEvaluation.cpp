/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../base/task_progress.h"
#include "../scripting/quickjs_script_engine.h"
#include "../scripting/quickjs_script_evaluation.h"

#include <quickjs.h>

namespace Mayo {

namespace {

JSValue jsDoSomething(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = QuickJsScriptEngine::getEvaluationState(context);
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress*) {
        /* Task succeeds immediately */
    }).release();
}

JSValue jsDoSomethingFailingTask(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = QuickJsScriptEngine::getEvaluationState(context);
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress*) {
        throw std::runtime_error("Expected test failure");
    }).release();
}

JSValue jsDoProgressTask(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = QuickJsScriptEngine::getEvaluationState(context);
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress* progress) {
        progress->setValue(10);
        progress->setValue(20);
    }).release();
}

JSValue jsWaitAbortMax5secTask(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = QuickJsScriptEngine::getEvaluationState(context);
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress* progress) {
        // Keep the task alive until cancellation, with a timeout as a safety net.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (!progress->isAbortRequested() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!progress->isAbortRequested())
            throw std::runtime_error("Task was not cancelled");
    }).release();
}

void installTestApi(JSContext* context, QuickJsScriptEvaluation& evaluation)
{
    QuickJsValue global{ context, JS_GetGlobalObject(context) };

    QuickJsValue test = QuickJsValue::newObject(context);

    test.setProperty(
        "doSomething",
        QuickJsValue::newFunction(context, jsDoSomething, "doSomething")
    );
    test.setProperty(
        "doSomethingFailingTask",
        QuickJsValue::newFunction(context, jsDoSomethingFailingTask, "doSomethingFailingTask")
    );
    test.setProperty(
        "doProgressTask",
        QuickJsValue::newFunction(context, jsDoProgressTask, "doProgressTask")
    );
    test.setProperty(
        "jsWaitAbortMax5secTask",
        QuickJsValue::newFunction(context, jsWaitAbortMax5secTask, "jsWaitAbortMax5secTask")
    );

    global.setProperty("test", std::move(test));
}

} // namespace

void TestScripting::QuickJsScriptEvaluation_startTaskResolvesPromise_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        let completed = false;
        await test.doSomething();
        completed = true;
        export default completed;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<bool>(eval.result.value), true);
}

void TestScripting::QuickJsScriptEvaluation_failingTaskReportsError_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        await test.doSomethingFailingTask();
    )");

    QVERIFY(!eval.result.success);

    const auto ptrMessage = eval.lastMessage(MessageType::Error);
    QVERIFY(ptrMessage);
    QVERIFY(ptrMessage->text.find("Task failed") != std::string::npos);
}

void TestScripting::QuickJsScriptEvaluation_startTaskSupportsMultiplePendingTasks_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const a = test.doSomething();
        const b = test.doSomething();
        await Promise.all([a, b]);
        export default true;
    )");

    QVERIFY(eval.result.success);
    QCOMPARE(std::any_cast<bool>(eval.result.value), true);
}

void TestScripting::QuickJsScriptEvaluation_startTaskRejectsPromiseOnTaskFailure_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        let caughtMessage = "";
        try {
            await test.doSomethingFailingTask();
        }
        catch (error) {
            caughtMessage = String(error);
        }
        export default caughtMessage;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<std::string>(eval.result.value), "Error: Task failed");
}

void TestScripting::QuickJsScriptEvaluation_startTaskReturnsPromise_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const task = test.doSomething();
        export default typeof task.then === "function";
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<bool>(eval.result.value), true);
}

void TestScripting::QuickJsScriptEvaluation_taskThenPropagatesValue_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const value = await test.doSomething().then(() => 42);
        export default value;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<double>(eval.result.value), 42.);
}

void TestScripting::QuickJsScriptEvaluation_taskOnProgressRegisterDuringCallback_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        let sequence = "";

        const task = test.doProgressTask();
        task.onProgress(pct => {
            sequence += `${pct},`;
            task.onProgress(pct => { sequence += `${pct},`; });
        });

        await task;
        export default sequence;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<std::string>(eval.result.value).c_str(), "10,20,20,100,100,100,");
}

void TestScripting::QuickJsScriptEvaluation_taskCancelRejectsPromise_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const task = test.jsWaitAbortMax5secTask();
        task.cancel();

        let caughtMessage = "";

        try {
            await task;
        }
        catch (error) {
            caughtMessage = String(error);
        }

        export default caughtMessage;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<std::string>(eval.result.value), "Error: Task aborted");
}

void TestScripting::QuickJsScriptEvaluation_taskOnProgressCallbackThrows_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const values = [];

        const task = test.doProgressTask();

        task.onProgress(() => { throw new Error("progress callback failure"); });
        task.onProgress(value => { values.push(value); });

        await task;

        export default values.slice(0, 2).join(",");
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QCOMPARE(std::any_cast<std::string>(eval.result.value).c_str(), "10,20");
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
    QCOMPARE(eval.lastMessage(MessageType::Error)->text.c_str(), "progress callback failure");
}

void TestScripting::QuickJsScriptEvaluation_taskCancelAfterCompletion_test()
{
    QuickJsScriptEngine engine;
    engine.addPreEvaluateCallback(&installTestApi);

    auto eval = evaluateScript(engine, R"(
        const task = test.doSomething();
        const result = await task;

        // Task finished, calling cancel() must not alter result
        task.cancel();

        export default result;
    )");

    QVERIFY(eval.result.success);
    QVERIFY(eval.result.value.has_value());
    QVERIFY(std::any_cast<bool>(eval.result.value));
}

} // namespace Mayo
