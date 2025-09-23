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

    // Asynchronous execution of script file defined with setScriptFilePath()
    // Does nothing(returns) if script execution is currently running
    void runEvaluate();
    void stopEvaluate();
    void runOrStopEvaluate();

    bool isEvaluateRunning() const;

    bool waitForEvaluateEnd(int msecs = -1);

    // Generates a critical message to the logging framework if the JS value owns an error(QJSValue::isError())
    static void logError(const QJSValue& jsVal, const char* functionName = nullptr);

    // Overload of the above function, but an error is created using the provided JS engine
    static void logError(QJSEngine* jsEngine, std::string_view message, const char* functionName = nullptr);

    struct Message {
        MessageType type = MessageType::Trace;
        std::string text;
        std::string contextFile;
        int contextLine = -1;
        // contextFunction?
    };

    enum class EndReason {
        Finished, Stopped
    };

    Signal<Message> signalMessage;
    Signal<> signalEvaluateStarted;
    Signal<EndReason> signalEvaluateEnded;

private:
    static void init(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env);

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
};

} // namespace Mayo
