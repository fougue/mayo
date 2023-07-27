/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "occ_progress_indicator.h"
#include "string_conv.h"
#include "task_progress.h"

#include <algorithm>

namespace Mayo {

OccProgressIndicator::OccProgressIndicator(TaskProgress* progress)
    : m_progress(progress)
{
#if OCC_VERSION_HEX < OCC_VERSION_CHECK(7, 5, 0)
    this->SetScale(0., 100., 1.);
#endif
}

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
void OccProgressIndicator::Show(const Message_ProgressScope& scope, const bool isForce)
{
    if (m_progress) {
        if (scope.Name() && (scope.Name() != m_lastStepName || isForce)) {
            m_progress->setStep(scope.Name());
            m_lastStepName = scope.Name();
        }

        const double pc = this->GetPosition(); // Always within [0,1]
        const int val = std::clamp(static_cast<int>(pc * 100), 0, 100);
        if (m_lastProgress != val || isForce) {
            m_progress->setValue(val);
            m_lastProgress = val;
        }
    }
}
#else
bool OccProgressIndicator::Show(const bool /*force*/)
{
    if (m_progress) {
        m_progress->setStep(to_stdStringView(this->GetScope(1).GetName()));
        const double pc = this->GetPosition(); // Always within [0,1]
        m_progress->setValue(pc * 100);
    }

    return true;
}
#endif

bool OccProgressIndicator::UserBreak()
{
    return TaskProgress::isAbortRequested(m_progress);
}

} // namespace Mayo
