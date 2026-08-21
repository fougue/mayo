/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/text_id.h"

#include <RWMesh_CoordinateSystem.hxx>

namespace Mayo { class Enumeration; }

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

    static const Enumeration& enum_RWMesh_CoordinateSystem();
    static const Enumeration& enum_LengthUnit();
};

} // namespace Mayo::IO
