/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "dxf_format_common.h"

struct Dxf_STYLE {
    // Code: 2
    DxfStringRef name;
    // Code: 40
    double fixedTextHeight = 0;
    // Code: 41
    double widthFactor = 1.;
    // Code: 50
    // AutoCad documentation doesn't specify units, but "Group Codes in Numerical Order" section
    // states that codes 50-58 are in degrees
    double obliqueAngle = 0.;
    // Code: 3
    DxfStringRef primaryFontFileName;
    // Code: 4
    DxfStringRef bigFontFileName;

    // TODO Code 70(standard flag values)
    // TODO Code 71(text generation flags)
    // TODO Code 42(last height used)
};

struct Dxf_LAYER {
    // Code: 2
    DxfStringRef name;
    // Code: 6
    DxfStringRef lineTypeName;
    // Code: 62
    DxfColorIndex colorId = dxfColorByLayer;
    // Code: 70
    unsigned flags = 0;
};
