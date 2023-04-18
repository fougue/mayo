/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <cstdint>

namespace Mayo {

// Task identifier type
using TaskId = uint64_t;

// Reserved value for null tasks
constexpr TaskId TaskId_null = UINT64_MAX;

// Syntactic sugar for task auto-deletion flag(see TaskManager::run/exec())
enum class TaskAutoDestroy { On, Off };

} // namespace Mayo
