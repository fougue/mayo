/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
