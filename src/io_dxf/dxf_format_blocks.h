/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "dxf_entity_variant.h"
#include "dxf_format_common.h"

#include <vector>

struct Dxf_BLOCK {
    // Code 2 or 3
    DxfStringRef name;
    // Code 8
    DxfStringRef layerName;
    // Code 10, 20, 30
    DxfCoords basePoint = {};
    // Code 70
    unsigned flags = 0;
    // Code 1
    DxfStringRef xrefPathName;
    // Code 4
    DxfStringRef description;

    // Contents
    std::vector<Dxf_EntityVariant> entities;
};
