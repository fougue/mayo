/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../scripting/quickjs_script_engine.h"

#include <quickjs.h>

namespace Mayo {

namespace {

JSValue jsDoSomething(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = static_cast<QuickJsScriptEvaluation*>(JS_GetContextOpaque(context));
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress*) {
        /* Task succeeds immediately */
    }).release();
}

JSValue jsDoSomethingFailingTask(JSContext* context, JSValueConst, int, JSValueConst*)
{
    auto evalState = static_cast<QuickJsScriptEvaluation*>(JS_GetContextOpaque(context));
    if (!evalState)
        return JS_ThrowInternalError(context, "QuickJsScriptEvaluation is not available");

    return evalState->startTask([](TaskProgress*) {
        throw std::runtime_error("Expected test failure");
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

} // namespace Mayo
