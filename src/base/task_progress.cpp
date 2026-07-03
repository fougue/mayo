/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "task_progress.h"
#include "task_manager.h"

#include "math_utils.h"

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

void TaskProgress::setValue(double pct)
{
    if (m_taskId == TaskId_null)
        return;

    if (m_isAbortRequested)
        return;

    const double valueOnEntry = m_value;
    pct = std::clamp(pct, 0., 100.);
    m_value = pct;

    if (m_parent) {
        const auto valueDeltaInParent = (m_value - valueOnEntry) * (m_portionSize / 100.);
        m_parent->setValue(m_parent->value() + valueDeltaInParent);
    }
    else {
        // Single decimal rounding function
        auto fnRound = [](double v) { return std::round(v * 10.) / 10.; };
        if (MathUtils::fuzzyIsNull(fnRound(pct))
            || !MathUtils::fuzzyEqual(fnRound(valueOnEntry), fnRound(pct)))
        {
            m_taskMgr->signalProgressChanged.send(m_taskId, m_value);
        }
    }
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
