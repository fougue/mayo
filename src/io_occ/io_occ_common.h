/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/enumeration_fromenum.h"

#include <RWMesh_CoordinateSystem.hxx>

namespace Mayo::IO {

class OccCommon {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccCommon)
public:
    enum class LengthUnit {
        Undefined = -1,
        Micrometer,
        Millimeter,
        Centimeter,
        Meter,
        Kilometer,
        Inch,
        Foot,
        Mile
    };

    static const char* toCafString(LengthUnit unit);
};

} // namespace Mayo::IO


namespace Mayo {

template<> struct EnumNames<IO::OccCommon::LengthUnit> {
    inline static std::string_view trContext = IO::OccCommon::textIdContext();
    inline static std::string_view junkPrefix = {};
};

template<> struct EnumNames<RWMesh_CoordinateSystem> {
    inline static std::string_view trContext = IO::OccCommon::textIdContext();
    inline static std::string_view junkPrefix = "RWMesh_CoordinateSystem_";
};

} // namespace Mayo
