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

namespace Mayo {

class QuickJsScriptEngine : public IScriptEngine {
public:
    ~QuickJsScriptEngine();

    void startEvaluate() override;
    void stopEvaluate() override;
    bool isEvaluateRunning() const override;
    bool waitForEvaluateEnd(int msecs = -1) override;

    bool isStopRequested() const;

private:
    void evaluateWorker(const std::string& script, const FilePath& scriptFilePath);

    mutable std::mutex m_mutex; // Protects m_isEvaluateRunning and m_evaluateThread
    std::condition_variable m_evaluateEndCondition;
    std::thread m_evaluateThread;
    bool m_isEvaluateRunning{false};
    std::atomic_bool m_stopRequested{false};
};

} // namespace Mayo
