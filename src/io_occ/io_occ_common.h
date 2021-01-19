/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_enumeration.h"

#include <RWMesh_CoordinateSystem.hxx>

namespace Mayo {
namespace IO {

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

} // namespace IO

template<> struct EnumNames<IO::OccCommon::LengthUnit> {
    inline static const QByteArray trContext = IO::OccCommon::textIdContext();
    inline static const std::string_view junkPrefix = "";
};

template<> struct EnumNames<RWMesh_CoordinateSystem> {
    inline static const QByteArray trContext = IO::OccCommon::textIdContext();
    inline static const std::string_view junkPrefix = "RWMesh_CoordinateSystem_";
};

} // namespace Mayo
