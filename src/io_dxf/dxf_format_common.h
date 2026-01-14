/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <string_view>

enum class DxfUnit {
    Unspecified = 0,   // Unspecified (No units)
    Inches,
    Feet,
    Miles,
    Millimeters,
    Centimeters,
    Meters,
    Kilometers,
    Microinches,
    Mils,
    Yards,
    Angstroms,
    Nanometers,
    Microns,
    Decimeters,
    Dekameters,
    Hectometers,
    Gigameters,
    AstronomicalUnits,
    LightYears,
    Parsecs
};

enum class DxfVersion {
    RUnknown,
    ROlder,
    R10,
    R11_12,
    R13,
    R14,
    R2000,
    R2004,
    R2007,
    R2010,
    R2013,
    R2018,
    RNewer
};

struct DxfCoords {
    double x; double y; double z;
};

struct DxfScale {
    double x; double y; double z;
};

using DxfStringRef = std::string_view;
using DxfColorIndex = int;
constexpr DxfColorIndex dxfColorByLayer = 256;
constexpr DxfColorIndex dxfColorByBlock = 0;
