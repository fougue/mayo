/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

// Auxiliary file to avoid OpenGL macros collisions between Qt and OpenCascade headers.

#include <map>
#include <string>
#include <variant>

namespace Mayo::Internal {

using InfoValue = std::variant<bool, int, double, std::string>;
std::map<std::string, InfoValue> getOccOpenGlInfos();

} // namespace Mayo::Internal
