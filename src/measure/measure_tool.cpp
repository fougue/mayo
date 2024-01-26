/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "measure_tool.h"

namespace Mayo {

MeasureValue IMeasureTool_computeValue(
        const IMeasureTool& tool, MeasureType type, const GraphicsOwnerPtr& owner
    )
{
    MeasureValue value;
    switch (type) {
    case MeasureType::VertexPosition:
        return tool.vertexPosition(owner);
    case MeasureType::CircleCenter:
    case MeasureType::CircleDiameter:
        return tool.circle(owner);
    case MeasureType::Length:
        return tool.length(owner);
    case MeasureType::Area:
        return tool.area(owner);
    case MeasureType::BoundingBox:
        return tool.boundingBox(owner);
    default:
        return value;
    } // endswitch
}

MeasureValue IMeasureTool_computeValue(
        const IMeasureTool& tool,
        MeasureType type,
        const GraphicsOwnerPtr& owner1,
        const GraphicsOwnerPtr& owner2
    )
{
    MeasureValue value;
    switch (type) {
    case MeasureType::MinDistance:
        return tool.minDistance(owner1, owner2);
    case MeasureType::CenterDistance:
        return tool.centerDistance(owner1, owner2);
    case MeasureType::Angle:
        return tool.angle(owner1, owner2);
    default:
        return value;
    } // endswitch
}

bool MeasureValue_isValid(const MeasureValue& value)
{
    return !std::holds_alternative<MeasureNone>(value);
}

} // namespace Mayo
