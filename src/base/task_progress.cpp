/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_progress.h"
#include "task.h"
#include "task_manager.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Mayo {

TaskProgress::TaskProgress()
{
}

TaskProgress::TaskProgress(TaskProgress* parent, double portionSize, const QString& step)
    : m_parent(parent),
      m_task(parent ? parent->m_task : nullptr),
      m_portionSize(std::clamp(portionSize, 0., 100.))
{
    if (!step.isEmpty())
        this->setStep(step);
}

TaskProgress::~TaskProgress()
{
    if (m_parent)
        this->setValue(100);
}

TaskId TaskProgress::taskId() const
{
    return m_task ? m_task->id() : std::numeric_limits<TaskId>::max();
}

TaskManager* TaskProgress::taskManager() const
{
    return m_task ? m_task->manager() : nullptr;
}

void TaskProgress::setValue(int pct)
{
    if (m_isAbortRequested)
        return;

    const int valueOnEntry = m_value;
    m_value = std::clamp(pct, 0, 100);
    if (m_value != 0 && m_value == valueOnEntry)
        return;

    if (m_parent) {
        const int valueDeltaInParent = std::round((m_value - valueOnEntry) * (m_portionSize / 100.));
        m_parent->setValue(m_parent->value() + valueDeltaInParent);
    }
    else {
        if (m_task)
            emit m_task->manager()->progressChanged(m_task->id(), m_value);
    }
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

void TaskProgress::requestAbort()
{
    m_isAbortRequested = true;
}

} // namespace Mayo
