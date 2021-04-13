/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"
#include "task_progress_portion.h"
#include <QtCore/QString>

namespace Mayo {

class Task;

class TaskProgress {
public:
    TaskProgress();

    TaskId taskId() const;

    // Value in [0,100]
    int value() const { return m_value; }

    const QString& step() const { return m_step; }

    bool isAbortRequested() const { return m_isAbortRequested; }
    static bool isAbortRequested(const TaskProgress* progress);
    static bool isAbortRequested(const TaskProgressPortion* progress);

    const TaskProgressPortion& rootPortion() const { return m_rootPortion; }
    TaskProgressPortion& rootPortion() { return m_rootPortion; }

    // Disable copy
    TaskProgress(const TaskProgress&) = delete;
    TaskProgress(TaskProgress&&) = delete;
    TaskProgress& operator=(const TaskProgress&) = delete;
    TaskProgress& operator=(TaskProgress&&) = delete;

private:
    void setValue(int pct);
    void setStep(const QString& title);
    void setTask(const Task* task);

    void requestAbort();

    friend class TaskManager;
    friend class TaskProgressPortion;

    const Task* m_task = nullptr;
    int m_value = 0;
    QString m_step;
    bool m_isAbortRequested = false;
    TaskProgressPortion m_rootPortion;
};

} // namespace Mayo
