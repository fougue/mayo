/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
