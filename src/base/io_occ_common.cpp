/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "io_occ_common.h"
#include "text_id.h"

#include <RWMesh_CoordinateSystem.hxx>

namespace Mayo {
namespace IO {

struct OccCommonI18N { MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::IO::OccCommonI18N) };

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

const Enumeration& OccCommon::enumMeshCoordinateSystem()
{
    static const Enumeration enumeration = {
        { RWMesh_CoordinateSystem_Undefined, OccCommonI18N::textId("SystemUndefined") },
        { RWMesh_CoordinateSystem_Zup, OccCommonI18N::textId("SystemPosZUp") },
        { RWMesh_CoordinateSystem_Yup, OccCommonI18N::textId("SystemPosYUp") }
    };
    return enumeration;
}

} // namespace IO
} // namespace Mayo
