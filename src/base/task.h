/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"

#include <functional>

namespace Mayo {

class TaskManager;
class TaskProgress;

using TaskJob = std::function<void(TaskProgress*)>;

class Task {
public:
    TaskId id() const { return m_id; }
    const TaskJob& job() const { return m_fn; }
    TaskManager* manager() const { return m_manager; }

private:
    friend class TaskManager;
    TaskId m_id = 0;
    TaskJob m_fn;
    TaskManager* m_manager = nullptr;
};

} // namespace Mayo
