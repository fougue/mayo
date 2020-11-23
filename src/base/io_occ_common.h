/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_enumeration.h"

namespace Mayo {
namespace IO {

class OccCommon {
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

    static const Enumeration& enumerationLengthUnit();
    static const Enumeration& enumMeshCoordinateSystem();
};

} // namespace IO
} // namespace Mayo
