/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "measure_tool.h"

class TopoDS_Shape;

namespace Mayo {

// Provides measurement services for BRep shapes
class MeasureToolBRep : public IMeasureTool {
public:
    Span<const GraphicsObjectSelectionMode> selectionModes(MeasureType type) const override;
    bool supports(const GraphicsObjectPtr& object) const override;
    bool supports(MeasureType type) const override;

    gp_Pnt vertexPosition(const GraphicsOwnerPtr& owner) const override;
    MeasureCircle circle(const GraphicsOwnerPtr& owner) const override;
    MeasureMinDistance minDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const override;
    MeasureAngle angle(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const override;
    QuantityLength length(const GraphicsOwnerPtr& owner) const override;
    QuantityArea area(const GraphicsOwnerPtr& owner) const override;

    static gp_Pnt brepVertexPosition(const TopoDS_Shape& shape);
    static MeasureCircle brepCircle(const TopoDS_Shape& shape);
    static MeasureMinDistance brepMinDistance(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static MeasureAngle brepAngle(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
};

} // namespace Mayo
