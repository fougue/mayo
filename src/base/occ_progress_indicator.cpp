/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "occ_progress_indicator.h"

namespace Mayo {

OccProgressIndicator::OccProgressIndicator(TaskProgress *progress)
    : m_progress(progress)
{
    this->SetScale(0., 100., 1.);
}

bool OccProgressIndicator::Show(const bool /*force*/)
{
    if (m_progress) {
        const Handle_TCollection_HAsciiString name = this->GetScope(1).GetName();
        if (!name.IsNull())
            m_progress->setStep(occ::QtUtils::fromUtf8ToQString(name->String()));

        const double pc = this->GetPosition(); // Always within [0,1]
        const int minVal = 0;
        const int maxVal = 100;
        const int val = minVal + pc * (maxVal - minVal);
        m_progress->setValue(val);
    }

    return true;
}

bool OccProgressIndicator::UserBreak()
{
    return TaskProgress::isAbortRequested(m_progress);
}

} // namespace Mayo
