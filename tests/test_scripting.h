/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../src/scripting/iscript_engine.h"

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include <vector>

namespace Mayo {

class TestScripting  : public QObject {
    Q_OBJECT
private slots:
    void QuickJsScriptEngine_evaluateRuntimeError_test();
    void QuickJsScriptEngine_evaluateSyntaxError_test();
    void QuickJsScriptEngine_evaluateScriptFile_test();
    void QuickJsScriptEngine_evaluateImportedModule_test();
    void QuickJsScriptEngine_evaluateImportedModuleConsoleContext_test();
    void QuickJsScriptEngine_evaluateMissingModule_test();
    void QuickJsScriptEngine_evaluateAwait_test();
    void QuickJsScriptEngine_evaluateRuntimeIsolation_test();
    void QuickJsScriptEngine_evaluateConsoleContext_test();
    void QuickJsScriptEngine_evaluateAsyncException_test();
    void QuickJsScriptEngine_evaluateModuleSyntaxError_test();
    void QuickJsScriptEngine_evaluateModuleRuntimeError_test();
    void QuickJsScriptEngine_evaluateNestedModules_test();
    void QuickJsScriptEngine_evaluateTwice_test();

    void QuickJsScriptEngine_evaluateValue_test();
    void QuickJsScriptEngine_evaluateValue_test_data();

    void QuickJsScriptEngine_consoleMessagesTypes_test();
    void QuickJsScriptEngine_consoleContextWithoutScriptFile_test();
    void QuickJsScriptEngine_startEvaluateWhileRunning_test();
    void QuickJsScriptEngine_stopEvaluate_test();
    void QuickJsScriptEngine_destroyWhileEvaluateRunning_test();

    void QuickJsPromise_promise_test();
    void QuickJsPromise_resolve_test();
    void QuickJsPromise_resolveUndefined_test();
    void QuickJsPromise_rejectValue_test();
    void QuickJsPromise_rejectMessage_test();
    void QuickJsPromise_resolveThenReject_test();
    void QuickJsPromise_rejectThenResolve_test();
    void QuickJsPromise_duplicatedPromiseReference_test();

    void QuickJsScriptEvaluation_startTaskResolvesPromise_test();
    void QuickJsScriptEvaluation_failingTaskReportsError_test();
    void QuickJsScriptEvaluation_startTaskSupportsMultiplePendingTasks_test();
    void QuickJsScriptEvaluation_startTaskRejectsPromiseOnTaskFailure_test();
    void QuickJsScriptEvaluation_startTaskReturnsPromise_test();

private:
    using JsEngineResult = IScriptEngine::Result;
    using JsEngineEndReason = IScriptEngine::EndReason;
    using JsEngineMessage = IScriptEngine::Message;

    struct ScriptEvaluation {
        JsEngineResult result;
        JsEngineEndReason endReason{JsEngineEndReason::Finished};
        bool waitEndSuccess{false};
        std::vector<JsEngineMessage> messages;

        const JsEngineMessage* lastMessage(MessageType msgType) const;
    };

    ScriptEvaluation evaluateScript(
        IScriptEngine& engine, std::string_view strScript, const FilePath& scriptFilePath = {}
    );
    ScriptEvaluation evaluateScript(
        std::string_view strScript, const FilePath& scriptFilePath = {}
    );
};

} // namespace Mayo
