/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "task_common.h"
#include <atomic>
#include <string>
#include <string_view>

namespace Mayo {

class TaskManager;

// Provides feedback on the progress of a running/executing task
class TaskProgress {
public:
    TaskProgress() = default;
    TaskProgress(TaskProgress* parent, double portionSize, std::string_view step = {});
    ~TaskProgress();

    static TaskProgress& null();
    bool isNull() const;

    TaskId taskId() const { return m_taskId; }
    TaskManager* taskManager() const { return m_taskMgr; }

    // Value in [0,100]
    double value() const { return m_value; }
    void setValue(double pct);

    const std::string& step() const { return m_step; }
    void setStep(std::string_view title);

    bool isRoot() const { return m_parent != nullptr; }
    const TaskProgress* parent() const { return m_parent; }
    TaskProgress* parent() { return m_parent; }

    bool isAbortRequested() const { return m_isAbortRequested; }
    static bool isAbortRequested(const TaskProgress* progress);

    // Disable copy
    TaskProgress(const TaskProgress&) = delete;
    TaskProgress(TaskProgress&&) = delete;
    TaskProgress& operator=(const TaskProgress&) = delete;
    TaskProgress& operator=(TaskProgress&&) = delete;

private:
    void setTaskId(TaskId id) { m_taskId = id; }
    void setTaskManager(TaskManager* mgr) { m_taskMgr = mgr; }
    void requestAbort();

    friend class TaskManager;

    TaskProgress* m_parent{nullptr};
    TaskManager* m_taskMgr{nullptr};
    TaskId m_taskId{TaskId_null};
    double m_portionSize{-1.};
    std::atomic<double> m_value = 0.;
    std::string m_step;
    bool m_isAbortRequested{false};
};

} // namespace Mayo
