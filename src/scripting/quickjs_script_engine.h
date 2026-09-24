/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string_view>
#include <thread>

struct JSRuntime;

namespace Mayo {

class QuickJsScriptEngine : public IScriptEngine {
public:
    ~QuickJsScriptEngine();

    void startEvaluate() override;
    void stopEvaluate() override;
    bool isEvaluateRunning() const override;
    bool waitForEvaluateEnd(int msecs) override;

private:
    void evaluateWorker(const FilePath& scriptFilePath);
    void emitMessage(
        MessageType type, std::string_view text, std::string_view contextFile, int contextLine = -1
    );
    static int interruptHandler(struct JSRuntime*, void* opaque);

    std::thread m_evaluateThread;
    std::atomic_bool m_isEvaluateRunning{false};
    std::atomic_bool m_stopRequested{false};
    std::condition_variable m_evaluateEndCondition;
    std::mutex m_evaluateEndMutex;
};

} // namespace Mayo
