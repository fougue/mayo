/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "tkernel_utils.h"
#include <Message_ProgressIndicator.hxx>

namespace Mayo {

class TaskProgress;

class OccProgressIndicator : public Message_ProgressIndicator {
public:
    OccProgressIndicator(TaskProgress* progress);

    bool UserBreak() override;

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    void Show (const Message_ProgressScope& theScope, const bool isForce) override;
#else
    bool Show(const bool force) override;
#endif

private:
    TaskProgress* m_progress = nullptr;
    const char* m_lastStepName = nullptr;
    int m_lastProgress = -1;
};

} // namespace Mayo
