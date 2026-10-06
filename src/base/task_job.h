/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <functional>

namespace Mayo {

class TaskProgress;

// Piece of code to be executed as a task(ie with TaskManager::run/exec())
using TaskJob = std::function<void(TaskProgress*)>;

} // namespace Mayo
