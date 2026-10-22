/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

namespace Mayo {

enum class MeasureType {
    None,
    VertexPosition,
    CircleCenter,
    CircleDiameter,
    MinDistance,
    MinDistanceToCenter,
    CenterDistance,
    Angle,
    Length,
    Area,
    BoundingBox
};

} // namespace Mayo
