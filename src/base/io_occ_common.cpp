/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "text_id.h"

namespace Mayo {
namespace IO {

struct OccCommonI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccCommonI18N) };

const Enumeration& OccCommon::enumerationLengthUnit()
{
    static const Enumeration enumeration = {
        { int(LengthUnit::Undefined), OccCommonI18N::textId("UnitUndefined"), {} },
        { int(LengthUnit::Micrometer), OccCommonI18N::textId("UnitMicrometer"), {} },
        { int(LengthUnit::Millimeter), OccCommonI18N::textId("UnitMillimeter"), {} },
        { int(LengthUnit::Centimeter), OccCommonI18N::textId("UnitCentimeter"), {} },
        { int(LengthUnit::Meter), OccCommonI18N::textId("UnitMeter"), {} },
        { int(LengthUnit::Kilometer), OccCommonI18N::textId("UnitKilometer"), {} },
        { int(LengthUnit::Inch), OccCommonI18N::textId("UnitInch"), {} },
        { int(LengthUnit::Foot), OccCommonI18N::textId("UnitFoot"), {} },
        { int(LengthUnit::Mile), OccCommonI18N::textId("UnitMile"), {} }
    };
    return enumeration;
}

} // namespace IO
} // namespace Mayo
