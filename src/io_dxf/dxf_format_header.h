/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "dxf_format_common.h"
#include <variant>

enum class DxfHeaderVariableValueType {
    Unknown = 0, Int, Double, String, Coords
};

using Dxf_HeaderVariableValue = std::variant<
    std::monostate, int, double, DxfStringRef, DxfCoords
>;

inline int dxfGetInt(const Dxf_HeaderVariableValue& value, int defaultValue = -1)
{
    if (std::holds_alternative<int>(value))
        return std::get<int>(value);
    else
        return defaultValue;
}

inline double dxfGetDouble(const Dxf_HeaderVariableValue& value, double defaultValue = 0.)
{
    if (std::holds_alternative<double>(value))
        return std::get<double>(value);
    else
        return defaultValue;
}

inline DxfStringRef dxfGetString(const Dxf_HeaderVariableValue& value)
{
    if (std::holds_alternative<DxfStringRef>(value))
        return std::get<DxfStringRef>(value);
    else
        return {};
}
