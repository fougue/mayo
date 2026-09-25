/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../src/scripting/quickjs_script_engine.h"

#include <fstream>

namespace Mayo {

namespace {

void writeTextFile(const FilePath& filePath, std::string_view contents)
{
    std::ofstream file(filePath, std::ios::binary);
    QVERIFY(file);
    file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    QVERIFY(file.good());
}

} // namespace

using JsEngineResult = IScriptEngine::Result;
using JsEngineEndReason = IScriptEngine::EndReason;

void TestScripting::evaluateNumber_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("42");

    JsEngineResult result;
    JsEngineEndReason endReason;
    engine.signalEvaluateEnded.connectSlot([&](const JsEngineResult& res, JsEngineEndReason reason) {
        result = res;
        endReason = reason;
    });

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QCOMPARE(endReason, JsEngineEndReason::Finished);
    QVERIFY(result.success);
    QVERIFY(result.value.has_value());
    QCOMPARE(std::any_cast<double>(result.value), 42.);
}

void TestScripting::evaluateString_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("'Hello Mayo'");

    JsEngineResult result;
    engine.signalEvaluateEnded.connectSlot([&](const JsEngineResult& res) { result = res; });

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QVERIFY(result.value.has_value());
    QCOMPARE(std::any_cast<std::string>(result.value), std::string{"Hello Mayo"});
}

void TestScripting::evaluateBoolean_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("true");

    JsEngineResult result;
    engine.signalEvaluateEnded.connectSlot([&](const JsEngineResult& res) { result = res; });

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QVERIFY(result.value.has_value());
    QCOMPARE(std::any_cast<bool>(result.value), true);
}

#if 0
void TestScripting::evaluateUndefined_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("undefined");

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QVERIFY(!result.value.has_value());
}

void TestScripting::evaluateNull_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("null");

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QVERIFY(!result.value.has_value());
}

void TestScripting::evaluateRuntimeError_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("throw new Error('Something went wrong')");

    IScriptEngine::Result result;
    IScriptEngine::Message message;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalMessage,
        this,
        [&](const IScriptEngine::Message& m) {
            if (m.type == IScriptEngine::MessageType::Error)
                message = m;
        }
        );

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(!result.success);
    QCOMPARE(message.type, IScriptEngine::MessageType::Error);
    QVERIFY(QString::fromStdString(message.text).contains("Something went wrong"));
}

void TestScripting::evaluateSyntaxError_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("const =");

    IScriptEngine::Result result;
    IScriptEngine::Message message;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalMessage,
        this,
        [&](const IScriptEngine::Message& m) {
            if (m.type == IScriptEngine::MessageType::Error)
                message = m;
        }
        );

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(!result.success);
    QCOMPARE(message.type, IScriptEngine::MessageType::Error);
}

void TestScripting::evaluateScriptFile_test()
{
    const FilePath filePath =
        std::filesystem::temp_directory_path() / "mayo-test-scripting.js";

    writeTextFile(filePath, "21 * 2");

    QuickJsScriptEngine engine;
    engine.setScriptFilePath(filePath);

    QCOMPARE(engine.scriptFilePath(), filePath);
    QCOMPARE(engine.script(), std::string("21 * 2"));

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QCOMPARE(std::any_cast<double>(result.value), 42.);

    std::filesystem::remove(filePath);
}

void TestScripting::evaluateMissingScriptFile_test()
{
    const FilePath filePath =
        std::filesystem::temp_directory_path() / "mayo-nonexistent-script.js";

    std::filesystem::remove(filePath);

    QuickJsScriptEngine engine;
    engine.setScriptFilePath(filePath);

    IScriptEngine::Result result;
    IScriptEngine::Message message;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalMessage,
        this,
        [&](const IScriptEngine::Message& m) {
            if (m.type == IScriptEngine::MessageType::Error)
                message = m;
        }
        );

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(!result.success);
    QCOMPARE(message.type, IScriptEngine::MessageType::Error);
    QCOMPARE(
        QString::fromStdString(message.contextFile),
        QString::fromStdString(filePath.u8string())
        );
}

void TestScripting::consoleMessages_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("console.log('Hello', 42, true)");

    QList<IScriptEngine::Message> messages;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalMessage,
        this,
        [&](const IScriptEngine::Message& message) {
            messages.append(message);
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QCOMPARE(messages.size(), 1);
    QCOMPARE(messages[0].type, IScriptEngine::MessageType::Trace);
    QCOMPARE(
        QString::fromStdString(messages[0].text),
        QString("Hello 42 true")
        );
}

void TestScripting::evaluateImportedModule_test()
{
    const FilePath directory =
        std::filesystem::temp_directory_path() / "mayo-test-scripting";

    std::filesystem::create_directories(directory);

    const FilePath modulePath = directory / "foo.js";

    writeTextFile(modulePath, "export const value = 42;");

    QuickJsScriptEngine engine;
    engine.setScript(
        "import { value } from './foo.js';\n"
        "value;",
        directory / "main.js"
        );

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QCOMPARE(std::any_cast<double>(result.value), 42.);

    std::filesystem::remove_all(directory);
}

void TestScripting::evaluateImportedModuleConsoleContext_test()
{
    const FilePath directory =
        std::filesystem::temp_directory_path() / "mayo-test-scripting";

    std::filesystem::create_directories(directory);

    const FilePath modulePath = directory / "foo.js";

    writeTextFile(modulePath, "console.log('Hello from foo');");

    QuickJsScriptEngine engine;
    engine.setScript(
        "import './foo.js';",
        directory / "main.js"
        );

    IScriptEngine::Message message;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalMessage,
        this,
        [&](const IScriptEngine::Message& m) {
            if (m.type == IScriptEngine::MessageType::Trace)
                message = m;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QCOMPARE(
        QString::fromStdString(message.contextFile),
        QString::fromStdString(modulePath.u8string())
        );

    std::filesystem::remove_all(directory);
}

void TestScripting::evaluateMissingModule_test()
{
    const FilePath directory =
        std::filesystem::temp_directory_path() / "mayo-test-scripting";

    std::filesystem::create_directories(directory);

    QuickJsScriptEngine engine;
    engine.setScript(
        "import './missing.js';",
        directory / "main.js"
        );

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(!result.success);

    std::filesystem::remove_all(directory);
}

void TestScripting::stopEvaluate_test()
{
    QuickJsScriptEngine engine;
    engine.setScript("while (true) {}");

    IScriptEngine::Result result;
    IScriptEngine::EndReason endReason = IScriptEngine::EndReason::Finished;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason reason) {
            result = r;
            endReason = reason;
        }
        );

    engine.startEvaluate();

    QTRY_VERIFY(engine.isEvaluateRunning());

    engine.stopEvaluate();

    QVERIFY(engine.waitForEvaluateEnd());

    QCOMPARE(endReason, IScriptEngine::EndReason::Stopped);
    QVERIFY(!result.success);
}

void TestScripting::evaluateTwice_test()
{
    QuickJsScriptEngine engine;

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.setScript("21");
    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QCOMPARE(std::any_cast<double>(result.value), 21.);

    engine.setScript("42");
    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QCOMPARE(std::any_cast<double>(result.value), 42.);
}

void TestScripting::evaluateRuntimeIsolation_test()
{
    QuickJsScriptEngine engine;

    IScriptEngine::Result result;

    QObject::connect(
        &engine,
        &QuickJsScriptEngine::signalEvaluateEnded,
        this,
        [&](const IScriptEngine::Result& r, IScriptEngine::EndReason) {
            result = r;
        }
        );

    engine.setScript("globalThis.testValue = 42;");
    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);

    engine.setScript("typeof globalThis.testValue");
    engine.startEvaluate();
    QVERIFY(engine.waitForEvaluateEnd());

    QVERIFY(result.success);
    QCOMPARE(
        std::any_cast<std::string>(result.value),
        std::string("undefined")
        );
}
#endif

} // namespace Mayo

QTEST_APPLESS_MAIN(Mayo::TestScripting)
