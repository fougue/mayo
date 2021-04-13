/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_progress_portion.h"
#include "task_progress.h"

#include <algorithm>
#include <cmath>

namespace Mayo {

TaskProgressPortion::TaskProgressPortion(TaskProgress& progress)
    : m_progress(&progress),
      m_portionSize(100)
{
}

TaskProgressPortion::TaskProgressPortion(
        TaskProgressPortion* parentPortion, int portionSize, const QString& step)
    : m_progress(parentPortion ? parentPortion->progress() : nullptr),
      m_parentPortion(parentPortion),
      m_portionSize(std::clamp(portionSize, 0, 100))
{
    if (!step.isEmpty())
        this->setStep(step);
}

TaskProgressPortion::~TaskProgressPortion()
{
    this->setValue(100);
}

void TaskProgressPortion::setValue(int pct)
{
    if (!m_progress || m_progress->isAbortRequested())
        return;

    const int valueOnEntry = m_value;
    m_value = std::clamp(pct, 0, 100);
    if (m_value != 0 && m_value == valueOnEntry)
        return;

    if (m_parentPortion) {
        const int valueDeltaInParent = std::round((m_value - valueOnEntry) * (m_portionSize / 100.));
        m_parentPortion->setValue(m_parentPortion->value() + valueDeltaInParent);
    }
    else {
        m_progress->setValue(m_value);
    }
}

void TaskProgressPortion::setStep(const QString& title)
{
    if (m_progress && !m_progress->isAbortRequested())
        m_progress->setStep(title);
}

bool TaskProgressPortion::isAbortRequested() const
{
    return m_progress && m_progress->isAbortRequested();
}

} // namespace Mayo
