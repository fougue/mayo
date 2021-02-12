/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "../base/text_id.h"

namespace Mayo {
namespace IO {

const char* OccCommon::toCafString(OccCommon::LengthUnit unit)
{
    switch (unit) {
    case LengthUnit::Undefined: return "??";
    case LengthUnit::Micrometer: return "UM";
    case LengthUnit::Millimeter: return "MM";
    case LengthUnit::Centimeter: return "CM";
    case LengthUnit::Meter: return "M";
    case LengthUnit::Kilometer: return "KM";
    case LengthUnit::Inch: return "INCH";
    case LengthUnit::Foot: return "FT";
    case LengthUnit::Mile: return "MI";
    }
    Q_UNREACHABLE();
}

} // namespace IO
} // namespace Mayo
