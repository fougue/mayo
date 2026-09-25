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
    void evaluateNumber_test();
    void evaluateString_test();
    void evaluateBoolean_test();
#if 0
    void evaluateUndefined_test();
    void evaluateNull_test();
    void evaluateRuntimeError_test();
    void evaluateSyntaxError_test();
    void evaluateScriptFile_test();
    void evaluateMissingScriptFile_test();
    void consoleMessages_test();
    void evaluateImportedModule_test();
    void evaluateImportedModuleConsoleContext_test();
    void evaluateMissingModule_test();
    void stopEvaluate_test();
    void evaluateTwice_test();
    void evaluateRuntimeIsolation_test();
#endif
};

} // namespace Mayo
