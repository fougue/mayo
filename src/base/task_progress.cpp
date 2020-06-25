/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_progress.h"
#include "task.h"
#include "task_manager.h"

namespace Mayo {

TaskProgress::TaskProgress(const Task& task)
    : m_task(&task)
{
}

TaskId TaskProgress::taskId() const
{
    assert(m_task != nullptr);
    return m_task->id();
}

void TaskProgress::setValue(int pct)
{
    if (m_currentScopeSize != -1)
        pct = m_currentScopeValueStart + pct * (m_currentScopeSize / 100.);

    m_value = pct;
    emit m_task->manager()->progressChanged(m_task->id(), pct);
}

void TaskProgress::setStep(const QString& title)
{
    m_step = title;
    emit m_task->manager()->progressStep(m_task->id(), title);
}

void TaskProgress::beginScope(int scopeSize, const QString &stepTitle)
{
    assert(m_currentScopeSize == -1);
    assert(scopeSize > 1);
    m_currentScopeSize = scopeSize;
    m_currentScopeValueStart = m_value;
    if (!stepTitle.isEmpty())
        this->setStep(stepTitle);
}

void TaskProgress::endScope()
{
    this->setValue(100);
    m_currentScopeSize = -1;
}

void TaskProgress::requestAbort()
{
    m_isAbortRequested = true;
}

} // namespace Mayo
