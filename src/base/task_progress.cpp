/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_progress.h"
#include "task_manager.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Mayo {

TaskProgress::TaskProgress(TaskProgress* parent, double portionSize, std::string_view step)
    : m_parent(parent),
      m_taskMgr(parent ? parent->m_taskMgr : nullptr),
      m_taskId(parent ? parent->m_taskId : TaskId_null),
      m_portionSize(std::clamp(portionSize, 0., 100.))
{
    if (!step.empty())
        this->setStep(step);
}

TaskProgress::~TaskProgress()
{
    if (m_parent)
        this->setValue(100);
}

TaskProgress& TaskProgress::null()
{
    static TaskProgress null;
    return null;
}

bool TaskProgress::isNull() const
{
    return m_taskId == TaskId_null;
}

void TaskProgress::setValue(int pct)
{
    if (m_taskId == TaskId_null)
        return;

    if (m_isAbortRequested)
        return;

    const int valueOnEntry = m_value;
    m_value = std::clamp(pct, 0, 100);
    if (m_value != 0 && m_value == valueOnEntry)
        return;

    if (m_parent) {
        const auto valueDeltaInParent = std::round((m_value - valueOnEntry) * (m_portionSize / 100.));
        m_parent->setValue(m_parent->value() + valueDeltaInParent);
    }
    else {
        m_taskMgr->signalProgressChanged.send(m_taskId, m_value);
    }
}

void TaskProgress::setValue(double pct)
{
    this->setValue(static_cast<int>(std::lround(pct)));
}

void TaskProgress::setStep(std::string_view title)
{
    if (m_taskMgr && m_taskId != TaskId_null) {
        m_step = title;
        m_taskMgr->signalProgressStep.send(m_taskId, m_step);
    }
}

bool TaskProgress::isAbortRequested(const TaskProgress* progress)
{
    return progress ? progress->isAbortRequested() : false;
}

void TaskProgress::requestAbort()
{
    if (m_taskId != TaskId_null)
        m_isAbortRequested = true;
}

} // namespace Mayo
