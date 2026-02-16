/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "dxf_format_common.h"

struct Dxf_STYLE {
    // Code 2
    DxfStringRef name;
    // Code 3
    DxfStringRef primaryFontFileName;
    // Code 4
    DxfStringRef bigFontFileName;
    // Code 40
    double fixedTextHeight = 0;
    // Code 41
    double widthFactor = 1.;
    // Code 42
    double lastHeightUsed = 0.;
    // Code 50
    double obliqueAngle = 0.; // degrees
    // Code 70
    //     1:  If set, this entry describes a shape
    //     4:  Vertical text
    //     16: If set, table entry is externally dependent on an xref
    //     32: If both this bit and bit 16 are set, the externally dependent xref has been
    //         successfully resolved
    //     64: If set, the table entry was referenced by at least one entity in the drawing the last
    //         time the drawing was edited. (This flag is for the benefit of AutoCADcommands. It can
    //         be ignored by most programs that read DXF files and need not be set by programs that
    //         write DXF files)
    unsigned standardFlags = 0;
    // Code 71  Text generation flags
    //   0: default
    //   2: text is backward(mirrored in X)
    //   4: text is upside down(mirrored in Y)
    unsigned generationFlags = 0;
};

struct Dxf_LAYER {
    // Code 2
    DxfStringRef name;
    // Code 6
    DxfStringRef lineTypeName;
    // Code 62
    DxfColorIndex colorId = dxfColorByLayer;
    // Code 70
    unsigned flags = 0;
};
