/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
