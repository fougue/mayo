/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "measure_tool.h"

class TopoDS_Edge;
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
    MeasureDistance minDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const override;
    MeasureDistance centerDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const override;
    MeasureAngle angle(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const override;
    MeasureLength length(const GraphicsOwnerPtr& owner) const override;
    MeasureArea area(const GraphicsOwnerPtr& owner) const override;
    MeasureBoundingBox boundingBox(const GraphicsOwnerPtr& owner) const override;

    static gp_Pnt brepVertexPosition(const TopoDS_Shape& shape);
    static MeasureCircle brepCircle(const TopoDS_Shape& shape);
    static MeasureDistance brepMinDistance(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static MeasureDistance brepCenterDistance(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static MeasureAngle brepAngle(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);
    static MeasureLength brepLength(const TopoDS_Shape& shape);
    static MeasureArea brepArea(const TopoDS_Shape& shape);
    static MeasureBoundingBox brepBoundingBox(const TopoDS_Shape& shape);

private:
    static MeasureCircle brepCircleFromGeometricEdge(const TopoDS_Edge& edge);
    static MeasureCircle brepCircleFromPolygonEdge(const TopoDS_Edge& edge);
};

} // namespace Mayo
