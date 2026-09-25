/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include "../base/filepath.h"
#include "../base/messenger.h"
#include "../base/signal.h"

#include <any>

namespace Mayo {

// Interface providing controlled execution of a JS script
class IScriptEngine {
public:
    virtual ~IScriptEngine() = default;

    // Script program to be executed
    const std::string& script() const { return m_script; }
    void setScript(std::string_view contents);

    // File path associated with the script program, if any
    const FilePath& scriptFilePath() const { return m_scriptFilePath; }
    void setScriptFilePath(const FilePath& filePath);

    // Asynchronous evaluation of script file defined with setScriptFilePath()
    // This function will return immediately after the script evaluation is started within a
    // background thread
    // Emits signalEvaluateStarted when the evaluation task has just started
    // Does nothing (returns) if script execution is currently running
    virtual void startEvaluate() = 0;

    // Requests interruption of any running script evaluation started with startEvaluate() and not
    // finished yet
    // Emits signalEvaluateEnded() with endReason==Stopped when the evaluation task has stopped
    virtual void stopEvaluate() = 0;

    // Depending on isEvaluateRunning() status, calls startEvaluate() or stopEvaluate()
    //     isEvaluateRunning  -> calls stopEvaluate()
    //     !isEvaluateRunning -> calls startEvaluate()
    void startOrStopEvaluate();

    // Returns true if a script evaluation is currently running (started with startEvaluate())
    virtual bool isEvaluateRunning() const = 0;

    // Blocks until the current script evaluation has finished/stopped and signalEvaluateEnded has
    // been emitted, or until `msecs` milliseconds have passed
    // If `msecs` is -1 then this function will not time out
    virtual bool waitForEvaluateEnd(int msecs = -1) = 0;

    // Payload data emitted by signalMessage
    struct Message {
        MessageType type = MessageType::Trace;
        std::string text;
        std::string contextFile;
        int contextLine = -1;
        // TODO Add contextFunction ?
    };

    // End reason of a script evaluation: completion (finished) or interruption (stopped)
    enum class EndReason {
        Finished, Stopped
    };

    // Result of a script evaluation, emitted by signalEvaluateEnded
    struct Result {
        bool success{false};
        std::any value; // TODO Replace by ScriptValue ? std::variant<monostate, bool, double, ...>
    };

    // Signal emitted when a JS console API function is called, such as `console.log()`
    Signal<Message> signalMessage;

    // Signal emitted when the evaluation of the script file has started
    Signal<> signalEvaluateStarted;

    // Signal emitted when the evaluation of the script file has ended (finished or stopped)
    Signal<Result, EndReason> signalEvaluateEnded;

private:
    std::string m_script;
    FilePath m_scriptFilePath; // Optional
};

} // namespace Mayo
