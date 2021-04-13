/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_progress.h"
#include "task.h"
#include "task_manager.h"

#include <limits>

namespace Mayo {

TaskProgress::TaskProgress()
    : m_rootPortion(*this)
{
}

TaskId TaskProgress::taskId() const
{
    return m_task ? m_task->id() : std::numeric_limits<TaskId>::max();
}

void TaskProgress::setValue(int pct)
{
    m_value = pct;
    if (m_task)
        emit m_task->manager()->progressChanged(m_task->id(), pct);
}

void TaskProgress::setStep(const QString& title)
{
    m_step = title;
    if (m_task)
        emit m_task->manager()->progressStep(m_task->id(), title);
}

void TaskProgress::setTask(const Task* task)
{
    m_task = task;
}

bool TaskProgress::isAbortRequested(const TaskProgress* progress)
{
    return progress ? progress->isAbortRequested() : false;
}

bool TaskProgress::isAbortRequested(const TaskProgressPortion* progress)
{
    return progress ? progress->isAbortRequested() : false;
}

void TaskProgress::requestAbort()
{
    m_isAbortRequested = true;
}

} // namespace Mayo
