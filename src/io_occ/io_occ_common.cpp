/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "io_occ_common.h"
#include "../base/meta_enum.h"
#include "../base/text_id.h"

#include <fmt/format.h>
#include <stdexcept>

namespace Mayo::IO {

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
    throw std::invalid_argument(fmt::format("{} isn't supported", MetaEnum::name(unit)));
}

} // namespace Mayo::IO
