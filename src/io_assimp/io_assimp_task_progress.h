/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <assimp/ProgressHandler.hpp>
namespace Mayo { class TaskProgress; }

namespace Mayo::IO {

// Provides assimp progress handler over Mayo::TaskProgress object
class AssimpTaskProgress : public Assimp::ProgressHandler {
public:
    AssimpTaskProgress(TaskProgress* progress);

    bool Update(float percent = -1.f) override;

private:
    TaskProgress* m_progress = nullptr;
};

} // namespace Mayo::IO
