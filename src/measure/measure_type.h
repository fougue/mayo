/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

namespace Mayo {

enum class MeasureType {
    None,
    VertexPosition,
    CircleCenter,
    CircleDiameter,
    MinDistance,
    CenterDistance,
    Angle,
    Length,
    Area,
    BoundingBox
};

} // namespace Mayo
