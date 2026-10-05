/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../src/scripting/quickjs_script_engine.h"

#include <gsl/util>
#include <fstream>

// Needed for Q_FECTH()
Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(std::any)

namespace Mayo {

namespace {

using JsEngineResult = IScriptEngine::Result;
using JsEngineEndReason = IScriptEngine::EndReason;
using JsEngineMessage = IScriptEngine::Message;

void writeTextFile(const FilePath& filePath, std::string_view contents)
{
    std::ofstream file(filePath, std::ios::binary);
    QVERIFY(file);
    file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    QVERIFY(file.good());
}

struct ScriptEvaluation {
    JsEngineResult result;
    JsEngineEndReason endReason{JsEngineEndReason::Finished};
    bool waitEndSuccess{false};
    std::vector<JsEngineMessage> messages;

    const JsEngineMessage* lastMessage(MessageType msgType) const
    {
        for (auto it = this->messages.rbegin(); it != this->messages.rend(); ++it) {
            if (it->type == msgType)
                return &(*it);
        }

        return nullptr;
    }
};

ScriptEvaluation evaluateScript(
        IScriptEngine& engine, std::string_view strScript, const FilePath& scriptFilePath = {}
    )
{
    engine.setScript(strScript);
    engine.setScriptFilePath(scriptFilePath);

    ScriptEvaluation evaluation;
    auto conn1 = engine.signalMessage.connectSlot([&](JsEngineMessage message) {
        evaluation.messages.push_back(std::move(message));
    });
    auto conn2 = engine.signalEvaluateEnded.connectSlot(
        [&](const JsEngineResult& result, JsEngineEndReason endReason) {
            evaluation.result = result;
            evaluation.endReason = endReason;
        }
        );
    auto _ = gsl::finally([&]{
        conn1.disconnect();
        conn2.disconnect();
    });

    engine.startEvaluate();
    evaluation.waitEndSuccess = engine.waitForEvaluateEnd();

    auto checkEnd = [&]{
        QVERIFY(evaluation.waitEndSuccess);
        QCOMPARE(evaluation.endReason, JsEngineEndReason::Finished);
    };
    checkEnd();

    return evaluation;
}

ScriptEvaluation evaluateScript(std::string_view strScript, const FilePath& scriptFilePath = {})
{
    QuickJsScriptEngine engine;
    return evaluateScript(engine, strScript, scriptFilePath);
}

} // namespace

void TestScripting::QuickJsScriptEngine_evaluateRuntimeError_test()
{
    auto eval = evaluateScript("throw new Error('Something went wrong')");

    QVERIFY(!eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
    QVERIFY(eval.lastMessage(MessageType::Error)->text.find("Something went wrong") != std::string::npos);
}

void TestScripting::QuickJsScriptEngine_evaluateSyntaxError_test()
{
    auto eval = evaluateScript("const =");

    QVERIFY(!eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
}

void TestScripting::QuickJsScriptEngine_evaluateScriptFile_test()
{
    const FilePath filePath = std::filesystem::temp_directory_path() / "mayo-test-scripting.js";
    const std::string strScript = "export default 21 * 2";

    QuickJsScriptEngine engine;
    auto eval = evaluateScript(engine, strScript, filePath);

    QCOMPARE(engine.scriptFilePath(), filePath);
    QCOMPARE(engine.script(), strScript);

    QVERIFY(eval.result.success);
    QCOMPARE(std::any_cast<double>(eval.result.value), 42.);

    std::filesystem::remove(filePath);
}

void TestScripting::QuickJsScriptEngine_consoleMessagesTypes_test()
{
    auto eval = evaluateScript(R"(
        console.log('log');
        console.info('info');
        console.warn('warn');
        console.error('error');
    )");

    QCOMPARE(eval.messages.size(), size_t(4));

    QCOMPARE(eval.messages[0].type, MessageType::Trace);
    QCOMPARE(eval.messages[1].type, MessageType::Info);
    QCOMPARE(eval.messages[2].type, MessageType::Warning);
    QCOMPARE(eval.messages[3].type, MessageType::Error);

    QCOMPARE(eval.messages[0].text, std::string{"log"});
    QCOMPARE(eval.messages[1].text, std::string{"info"});
    QCOMPARE(eval.messages[2].text, std::string{"warn"});
    QCOMPARE(eval.messages[3].text, std::string{"error"});
}

void TestScripting::QuickJsScriptEngine_evaluateImportedModule_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";
    std::filesystem::create_directories(directory);
    const FilePath modulePath = directory / "foo.js";
    writeTextFile(modulePath, "export const value = 42");

    std::string_view strScript = R"(
        import { value } from './foo.js';
        export default value;
    )";
    auto eval = evaluateScript(strScript, directory / "main.js");

    QVERIFY(eval.result.success);
    QCOMPARE(std::any_cast<double>(eval.result.value), 42.);

    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_evaluateImportedModuleConsoleContext_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";
    std::filesystem::create_directories(directory);

    const FilePath modulePath = directory / "foo.js";
    writeTextFile(modulePath, "console.log('Hello from foo')");

    auto eval = evaluateScript("import './foo.js'", directory / "main.js");

    QVERIFY(eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Trace) != nullptr);
    QCOMPARE(eval.lastMessage(MessageType::Trace)->contextFile, modulePath.u8string());

    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_evaluateMissingModule_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";
    std::filesystem::create_directories(directory);

    auto eval = evaluateScript("import './missing.js'", directory / "main.js");

    QVERIFY(!eval.result.success);
    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_stopEvaluate_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("while (true) {}");

    JsEngineResult result;
    JsEngineEndReason endReason = JsEngineEndReason::Finished;
    engine.signalEvaluateEnded.connectSlot([&](const JsEngineResult& res, JsEngineEndReason reason) {
        result = res;
        endReason = reason;
    });

    engine.startEvaluate();
    QTRY_VERIFY(engine.isEvaluateRunning());

    engine.stopEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QCOMPARE(endReason, JsEngineEndReason::Stopped);
    QVERIFY(!result.success);
}

void TestScripting::QuickJsScriptEngine_evaluateAwait_test()
{
    auto eval = evaluateScript("const value = await Promise.resolve(42)");
    QVERIFY(eval.result.success);
}

void TestScripting::QuickJsScriptEngine_evaluateRuntimeIsolation_test()
{
    QuickJsScriptEngine engine;

    {
        auto eval = evaluateScript(engine, "globalThis.testValue = 42");
        QVERIFY(eval.result.success);
    }

    {
        auto eval = evaluateScript(engine, "export default typeof globalThis.testValue");
        QVERIFY(eval.result.success);
        QCOMPARE(std::any_cast<std::string>(eval.result.value), std::string{"undefined"});
    }
}

void TestScripting::QuickJsScriptEngine_evaluateConsoleContext_test()
{
    const FilePath filePath = std::filesystem::temp_directory_path() / "main.js";

    auto eval = evaluateScript("console.log('Hello')", filePath);

    QVERIFY(eval.lastMessage(MessageType::Trace) != nullptr);
    QCOMPARE(eval.lastMessage(MessageType::Trace)->contextFile, filePath.u8string());
}

void TestScripting::QuickJsScriptEngine_consoleContextWithoutScriptFile_test()
{
    auto eval = evaluateScript("console.log('Hello')");

    QVERIFY(eval.lastMessage(MessageType::Trace) != nullptr);
    QCOMPARE(eval.lastMessage(MessageType::Trace)->contextFile, std::string{});
}

void TestScripting::QuickJsScriptEngine_evaluateAsyncException_test()
{
    auto eval = evaluateScript("await Promise.reject(new Error('Async failure'))");

    QVERIFY(!eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
    QVERIFY(eval.lastMessage(MessageType::Error)->text.find("Async failure") != std::string::npos);
}

void TestScripting::QuickJsScriptEngine_destroyWhileEvaluateRunning_test()
{
    {
        QuickJsScriptEngine engine;
        engine.setScript("while (true) {}");

        engine.startEvaluate();
        QTRY_VERIFY(engine.isEvaluateRunning());
    }

    // Reaching this point means the destructor successfully stopped and joined the evaluation thread
    QVERIFY(true);
}

void TestScripting::QuickJsScriptEngine_evaluateModuleSyntaxError_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";
    std::filesystem::create_directories(directory);

    const FilePath modulePath = directory / "foo.js";
    writeTextFile(modulePath, "export const = 42;");

    auto eval = evaluateScript("import './foo.js'", directory / "main.js");

    QVERIFY(!eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
    QVERIFY(!eval.lastMessage(MessageType::Error)->text.empty());

    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_evaluateModuleRuntimeError_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";
    std::filesystem::create_directories(directory);

    const FilePath modulePath = directory / "foo.js";
    writeTextFile(modulePath, "throw new Error('Module failure');");

    auto eval = evaluateScript("import './foo.js'", directory / "main.js");

    QVERIFY(!eval.result.success);
    QVERIFY(eval.lastMessage(MessageType::Error) != nullptr);
    QVERIFY(eval.lastMessage(MessageType::Error)->text.find("Module failure") != std::string::npos);

    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_evaluateNestedModules_test()
{
    const FilePath directory = std::filesystem::temp_directory_path() / "mayo-test-scripting";

    const FilePath subDirectory = directory / "sub";
    std::filesystem::create_directories(subDirectory);

    const FilePath mainPath = directory / "main.js";
    const FilePath fooPath = subDirectory / "foo.js";
    const FilePath barPath = directory / "bar.js";

    writeTextFile(barPath, "export default 42;");
    writeTextFile(fooPath, R"(
        import value from '../bar.js';
        export default value;
    )");

    std::string_view strScript = R"(
        import value from './sub/foo.js';
        export default value;
    )";
    auto eval = evaluateScript(strScript, mainPath);

    QVERIFY(eval.result.success);
    QCOMPARE(std::any_cast<double>(eval.result.value), 42.);

    std::filesystem::remove_all(directory);
}

void TestScripting::QuickJsScriptEngine_evaluateTwice_test()
{
    QuickJsScriptEngine engine;

    {
        auto eval = evaluateScript(engine, "export default 21");
        QVERIFY(eval.result.success);
        QCOMPARE(std::any_cast<double>(eval.result.value), 21.);
    }

    {
        auto eval = evaluateScript(engine, "export default 42");
        QVERIFY(eval.result.success);
        QCOMPARE(std::any_cast<double>(eval.result.value), 42.);
    }
}

void TestScripting::QuickJsScriptEngine_startEvaluateWhileRunning_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("while (true) {}");

    int evaluateStartedCount = 0;
    engine.signalEvaluateStarted.connectSlot([&]{ ++evaluateStartedCount; });

    engine.startEvaluate();

    QTRY_COMPARE(evaluateStartedCount, 1);

    engine.startEvaluate(); // Must be ignored

    QCOMPARE(evaluateStartedCount, 1);

    engine.stopEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());
}

void TestScripting::QuickJsScriptEngine_evaluateValue_test()
{
    QFETCH(std::string, strScriptValue);
    QFETCH(std::any, expectedAny);

    auto eval = evaluateScript("export default " + strScriptValue);

    QVERIFY(eval.result.success);
    QCOMPARE(eval.result.value.has_value(), expectedAny.has_value());
    QCOMPARE(eval.result.value.type(), expectedAny.type());

    if (!expectedAny.has_value())
        return;

    if (expectedAny.type() == typeid(double))
        QCOMPARE(std::any_cast<double>(eval.result.value), std::any_cast<double>(expectedAny));
    else if (expectedAny.type() == typeid(std::string))
        QCOMPARE(std::any_cast<std::string>(eval.result.value), std::any_cast<std::string>(expectedAny));
    else if (expectedAny.type() == typeid(bool))
        QCOMPARE(std::any_cast<bool>(eval.result.value), std::any_cast<bool>(expectedAny));
    else
        QFAIL("Unsupported expected std::any type");
}

void TestScripting::QuickJsScriptEngine_evaluateValue_test_data()
{
    QTest::addColumn<std::string>("strScriptValue");
    QTest::addColumn<std::any>("expectedAny");

    using namespace std::string_literals;
    QTest::newRow("int(42)") << "42"s << std::any{42.};
    QTest::newRow("int(-42)") << "-42"s << std::any{-42.};
    QTest::newRow("double(3.14159)") << "3.14159"s << std::any{3.14159};
    QTest::newRow("string('Hello Mayo')") << "'Hello Mayo'"s << std::any{"Hello Mayo"s};
    QTest::newRow("string('')") << "''"s << std::any{""s};
    QTest::newRow("bool(true)") << "true"s << std::any{true};
    QTest::newRow("bool(false)") << "false"s << std::any{false};
    QTest::newRow("undefined") << "undefined"s << std::any{};
    QTest::newRow("null") << "null"s << std::any{};
}

} // namespace Mayo
