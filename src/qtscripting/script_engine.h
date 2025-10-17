/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/filepath.h"
#include "../base/messenger.h"
#include "../base/signal.h"
#include "../base/task_manager.h"
#include "script_environment.h"

#include <QtCore/QJsonValue>
class QJSEngine;
class QJSValue;

namespace Mayo {

// Provides controlled execution of a JS script
class ScriptEngine {
public:
    ScriptEngine(const ApplicationPtr& mainApp, const ScriptEnvironment& env);

    // Path to the script file to be executed
    FilePath scriptFilePath() const { return m_scriptFilePath; }
    void setScriptFilePath(const FilePath& filePath);

    // Asynchronous evaluation of script file defined with setScriptFilePath()
    // This function will return immediately after the script evaluation is started within a
    // background thread
    // Emits signalEvaluateStarted when the evaluation task has just started
    // Does nothing(returns) if script execution is currently running
    void runEvaluate();

    // Stops(interrupts) any running script evaluation started with runEvaluate() and not
    // finished yet
    // Emits signalEvaluateEnded() with endReason==EndReason::Stopped when the evaluation task has
    // stopped
    void stopEvaluate();

    // Depending on isEvaluateRunning() status, calls runEvaluate() or stopEvaluate()
    //     isEvaluateRunning  -> calls stopEvaluate()
    //     !isEvaluateRunning -> calls runEvaluate()
    void runOrStopEvaluate();

    // Returns true if a script evaluation is currently running(started with runEvaluate())
    bool isEvaluateRunning() const;

    // Blocks until the current script evaluation has finished/stopped and signalEvaluateEnded has
    // been emitted, or until `msecs` milliseconds have passed
    // If `msecs` is -1, this function will not time out.
    bool waitForEvaluateEnd(int msecs = -1);

    // Payload data emitted by signalMessage
    struct Message {
        MessageType type = MessageType::Trace;
        std::string text;
        std::string contextFile;
        int contextLine = -1;
        // contextFunction?
    };

    // End reason of a script evaluation: completion(finished) or interruption(stopped)
    enum class EndReason {
        Finished, Stopped
    };

    // Result of a script evaluation, emitted by signalEvaluateEnded
    struct Result {
        bool success = false;
        QJsonValue value;
    };

    // Signal emitted when a JS console API function is called, such as `console.log()`
    Signal<Message> signalMessage;

    // Signal emitted when the evaluation of the script file has started
    Signal<> signalEvaluateStarted;

    // Signal emitted when the evaluation of the script file has ended(finished or stopped)
    Signal<Result, EndReason> signalEvaluateEnded;

    // -- Helper functions

    // Configures the JS engine so it can support Mayo Scripting API
    static void init(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env);

    // Generates a critical message to the logging framework if the JS value owns an error(QJSValue::isError())
    static void logError(const QJSValue& jsVal, const char* functionName = nullptr);

    // Overload of the above function, but an error is created using the provided JS engine
    static void logError(QJSEngine* jsEngine, std::string_view message, const char* functionName = nullptr);

private:
    void onTaskStarted(TaskId taskId);
    void onTaskEnded(TaskId taskId);

    ApplicationPtr m_mainApp;
    ScriptEnvironment m_scriptEnv;
    QJSEngine* m_jsEngine = nullptr;
    FilePath m_scriptFilePath;
    TaskManager m_taskMgr;
    TaskId m_scriptExecTaskId = TaskId_null;
    bool m_wasEvaluateStopped = false;
    bool m_isEvaluateRunning = false;
    Result m_evalResult;
};

} // namespace Mayo
