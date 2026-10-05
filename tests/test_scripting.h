/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

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
};

} // namespace Mayo
