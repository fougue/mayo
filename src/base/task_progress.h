/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"
#include <atomic>
#include <string>
#include <string_view>

namespace Mayo {

class Task;
class TaskManager;

// Provides feedback on the progress of a running/executing task
class TaskProgress {
public:
    TaskProgress();
    TaskProgress(TaskProgress* parent, double portionSize, std::string_view step = {});
    ~TaskProgress();

    static TaskProgress& null();
    bool isNull() const;

    TaskId taskId() const;
    TaskManager* taskManager() const;

    // Value in [0,100]
    int value() const { return m_value; }
    void setValue(int pct);

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
    void setTask(const Task* task);
    void requestAbort();

    friend class TaskManager;

    TaskProgress* m_parent = nullptr;
    const Task* m_task = nullptr;
    double m_portionSize = -1;
    std::atomic<int> m_value = 0;
    std::string m_step;
    bool m_isAbortRequested = false;
};

} // namespace Mayo
