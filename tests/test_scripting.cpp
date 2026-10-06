/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../src/scripting/quickjs_script_engine.h"

#include <gsl/util>

namespace Mayo {

const TestScripting::JsEngineMessage*
TestScripting::ScriptEvaluation::lastMessage(MessageType msgType) const
{
    for (auto it = this->messages.rbegin(); it != this->messages.rend(); ++it) {
        if (it->type == msgType)
            return &(*it);
    }

    return nullptr;
}

TestScripting::ScriptEvaluation TestScripting::evaluateScript(
        IScriptEngine& engine, std::string_view strScript, const FilePath& scriptFilePath
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

TestScripting::ScriptEvaluation TestScripting::evaluateScript(
        std::string_view strScript, const FilePath& scriptFilePath
    )
{
    QuickJsScriptEngine engine;
    return TestScripting::evaluateScript(engine, strScript, scriptFilePath);
}

} // namespace Mayo

// Qt application needed for QTRY_VERIFY(), QTRY_COMPARE(), ...
QTEST_MAIN(Mayo::TestScripting)
//QTEST_APPLESS_MAIN(Mayo::TestScripting)
