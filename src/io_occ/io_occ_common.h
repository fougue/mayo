/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_enumeration.h"
#include "../base/tkernel_utils.h"

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <RWMesh_CoordinateSystem.hxx>
#endif

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
    inline static std::string_view junkPrefix = "";
};

#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
template<> struct EnumNames<RWMesh_CoordinateSystem> {
    inline static std::string_view trContext = IO::OccCommon::textIdContext();
    inline static std::string_view junkPrefix = "RWMesh_CoordinateSystem_";
};
#endif

} // namespace Mayo
