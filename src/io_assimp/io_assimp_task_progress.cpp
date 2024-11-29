/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_assimp_task_progress.h"

#include "../base/task_progress.h"

namespace Mayo::IO {

AssimpTaskProgress::AssimpTaskProgress(TaskProgress* progress)
    : m_progress(progress)
{
}

bool AssimpTaskProgress::Update(float percent)
{
    if (TaskProgress::isAbortRequested(m_progress))
        return false;

    if (percent > 0)
        m_progress->setValue(percent * 100);

    return true;
}

} // namespace Mayo::IO
