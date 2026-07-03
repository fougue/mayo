/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "tkernel_utils.h"
#include <Message_ProgressIndicator.hxx>

namespace Mayo {

class TaskProgress;

// Provides implementation of OpenCascade-based progress indicator around Mayo::TaskProgress
class OccProgressIndicator : public Message_ProgressIndicator {
public:
    explicit OccProgressIndicator(TaskProgress* progress);

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
