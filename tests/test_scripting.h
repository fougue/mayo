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
    void evaluateRuntimeError_test();
    void evaluateSyntaxError_test();
    void evaluateScriptFile_test();
    void evaluateImportedModule_test();
    void evaluateImportedModuleConsoleContext_test();
    void evaluateMissingModule_test();
    void evaluateAwait_test();
    void evaluateRuntimeIsolation_test();
    void evaluateConsoleContext_test();
    void evaluateAsyncException_test();
    void evaluateModuleSyntaxError_test();
    void evaluateModuleRuntimeError_test();
    void evaluateNestedModules_test();
    void evaluateTwice_test();

    void evaluateValue_test();
    void evaluateValue_test_data();

    void consoleMessagesTypes_test();
    void consoleContextWithoutScriptFile_test();
    void startEvaluateWhileRunning_test();
    void stopEvaluate_test();
    void destroyWhileEvaluateRunning_test();
};

} // namespace Mayo
