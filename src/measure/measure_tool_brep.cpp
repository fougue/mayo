/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "measure_tool_brep.h"

#include "../base/geom_utils.h"
#include "../base/text_id.h"
#include "../graphics/graphics_shape_object_driver.h"

#include <gp_Elips.hxx>
#include <AIS_Shape.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <GC_MakeCircle.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <GProp_GProps.hxx>
#include <Precision.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070500
#  include <PrsDim_AngleDimension.hxx>
#else
#  include <AIS_AngleDimension.hxx>
using PrsDim_AngleDimension = AIS_AngleDimension;
#endif

#include <cmath>
#include <optional>

namespace Mayo {

namespace {

enum class ErrorCode {
    Unknown,
    NotVertex,
    NotCircularEdge,
    NotBRepShape,
    NotGeometricOrPolygonEdge,
    MinDistanceFailure,
    NotAllEdges,
    NotLinearEdge,
    NotAllFaces,
    ParallelEdges
};

template<ErrorCode Err>
class BRepMeasureError : public IMeasureError {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::BRepMeasureError)
public:
    std::string_view message() const override
    {
        switch (Err) {
        case ErrorCode::NotVertex:
            return textIdTr("Entity must be a vertex");
        case ErrorCode::NotCircularEdge:
            return textIdTr("Entity must be a circular edge");
        case ErrorCode::NotBRepShape:
            return textIdTr("Entity must be a shape(BREP)");
        case ErrorCode::NotGeometricOrPolygonEdge:
            return textIdTr("Entity must be a geometric or polygon edge");
        case ErrorCode::MinDistanceFailure:
            return textIdTr("Computation of minimum distance failed");
        case ErrorCode::NotAllEdges:
            return textIdTr("All entities must be edges");
        case ErrorCode::NotLinearEdge:
            return textIdTr("Entity must be a linear edge");
        case ErrorCode::NotAllFaces:
            return textIdTr("All entities must be faces");
        case ErrorCode::ParallelEdges:
            return textIdTr("Entities must not be parallel");
        default:
            return textIdTr("Unknown error");
        }
    }
};

template<ErrorCode Err> void throwErrorIf(bool cond)
{
    if (cond)
        throw BRepMeasureError<Err>();
}

const TopoDS_Shape getShape(const GraphicsOwnerPtr& owner)
{
    static const TopoDS_Shape nullShape;
    auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(owner);
    return brepOwner ? brepOwner->Shape().Moved(owner->Location()) : nullShape;
}

} // namespace

Span<const GraphicsObjectSelectionMode> MeasureToolBRep::selectionModes(MeasureType type) const
{
    switch (type) {
    case MeasureType::VertexPosition: {
        static const GraphicsObjectSelectionMode modes[] = { AIS_Shape::SelectionMode(TopAbs_VERTEX) };
        return modes;
    }
    case MeasureType::CircleCenter:
    case MeasureType::CircleDiameter:
    case MeasureType::Length:
    case MeasureType::Angle: {
        static const GraphicsObjectSelectionMode modes[] = { AIS_Shape::SelectionMode(TopAbs_EDGE) };
        return modes;
    }
    case MeasureType::MinDistance: {
        static const GraphicsObjectSelectionMode modes[] = {
            AIS_Shape::SelectionMode(TopAbs_VERTEX),
            AIS_Shape::SelectionMode(TopAbs_EDGE),
            AIS_Shape::SelectionMode(TopAbs_FACE)
        };
        return modes;
    }
    case MeasureType::Area: {
        static const GraphicsObjectSelectionMode modes[] = { AIS_Shape::SelectionMode(TopAbs_FACE) };
        return modes;
    }
    } // endswitch

    return {};
}

bool MeasureToolBRep::supports(const GraphicsObjectPtr& object) const
{
    auto gfxDriver = GraphicsObjectDriver::get(object);
    return gfxDriver ? !GraphicsShapeObjectDriverPtr::DownCast(gfxDriver).IsNull() : false;
}

bool MeasureToolBRep::supports(MeasureType type) const
{
    return type != MeasureType::None;
}

gp_Pnt MeasureToolBRep::vertexPosition(const GraphicsOwnerPtr& owner) const
{
    return brepVertexPosition(getShape(owner));
}

MeasureCircle MeasureToolBRep::circle(const GraphicsOwnerPtr& owner) const
{
    return brepCircle(getShape(owner));
}

MeasureMinDistance MeasureToolBRep::minDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const
{
    return brepMinDistance(getShape(owner1), getShape(owner2));
}

MeasureAngle MeasureToolBRep::angle(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const
{
    return brepAngle(getShape(owner1), getShape(owner2));
}

QuantityLength MeasureToolBRep::length(const GraphicsOwnerPtr& owner) const
{
    return brepLength(getShape(owner));
}

QuantityArea MeasureToolBRep::area(const GraphicsOwnerPtr& owner) const
{
    const TopoDS_Shape shape = getShape(owner);
    throwErrorIf<ErrorCode::NotAllFaces>(shape.IsNull() || shape.ShapeType() != TopAbs_FACE);
    GProp_GProps gprops;
    BRepGProp::SurfaceProperties(TopoDS::Face(shape), gprops);
    const double area = gprops.Mass();
    return area * Quantity_SquareMillimeter;
}

gp_Pnt MeasureToolBRep::brepVertexPosition(const TopoDS_Shape& shape)
{
    throwErrorIf<ErrorCode::NotVertex>(shape.IsNull() || shape.ShapeType() != TopAbs_VERTEX);
    return BRep_Tool::Pnt(TopoDS::Vertex(shape));
}

MeasureCircle MeasureToolBRep::brepCircle(const TopoDS_Shape& shape)
{
    auto fnThrowErrorIf = [](bool cond) {
        throwErrorIf<ErrorCode::NotCircularEdge>(cond);
    };

    fnThrowErrorIf(shape.IsNull() || shape.ShapeType() != TopAbs_EDGE);

    const BRepAdaptor_Curve curve(TopoDS::Edge(shape));
    std::optional<gp_Circ> circle;
    if (curve.GetType() == GeomAbs_Circle) {
        circle = curve.Circle();
    }
    else if (curve.GetType() == GeomAbs_Ellipse) {
        const gp_Elips ellipse  = curve.Ellipse();
        if (std::abs(ellipse.MinorRadius() - ellipse.MajorRadius()) < Precision::Confusion())
            circle = gp_Circ{ ellipse.Position(), ellipse.MinorRadius() };
    }
    else {
        // Try to create a circle from 3 sample points on the curve
        {
            const GCPnts_QuasiUniformAbscissa pnts(curve, 4); // More points to avoid confusion
            fnThrowErrorIf(!pnts.IsDone() || pnts.NbPoints() < 3);
            const GC_MakeCircle makeCirc(
                        GeomUtils::d0(curve, pnts.Parameter(1)),
                        GeomUtils::d0(curve, pnts.Parameter(2)),
                        GeomUtils::d0(curve, pnts.Parameter(3))
                        );
            fnThrowErrorIf(!makeCirc.IsDone());
            circle = makeCirc.Value()->Circ();
        }

        // Take more sample points on the curve and check for each that:
        //     dist(pntSample, pntCircleCenter) - circleRadius < tolerance
        {
            const GCPnts_QuasiUniformAbscissa pnts(curve, 64);
            fnThrowErrorIf(!pnts.IsDone());
            for (int i = 1; i <= pnts.NbPoints(); ++i) {
                const gp_Pnt pntSample = GeomUtils::d0(curve, pnts.Parameter(i));
                const double dist = pntSample.Distance(circle->Location());
                fnThrowErrorIf(std::abs(dist - circle->Radius()) > 1e-4);
            }
        }
    }

    fnThrowErrorIf(!circle);
    MeasureCircle result;
    result.pntAnchor = GeomUtils::d0(curve, curve.FirstParameter());
    result.isArc = !curve.IsClosed();
    result.value = circle.value();
    return result;
}

MeasureMinDistance MeasureToolBRep::brepMinDistance(
        const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    throwErrorIf<ErrorCode::NotBRepShape>(shape1.IsNull());
    throwErrorIf<ErrorCode::NotBRepShape>(shape2.IsNull());

    const BRepExtrema_DistShapeShape dist(shape1, shape2);
    throwErrorIf<ErrorCode::MinDistanceFailure>(!dist.IsDone());

    MeasureMinDistance distResult;
    distResult.pnt1 = dist.PointOnShape1(1);
    distResult.pnt2 = dist.PointOnShape2(1);
    distResult.value = dist.Value() * Quantity_Millimeter;
    return distResult;
}

MeasureAngle MeasureToolBRep::brepAngle(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    MeasureAngle angleResult;

    throwErrorIf<ErrorCode::NotBRepShape>(shape1.IsNull());
    throwErrorIf<ErrorCode::NotBRepShape>(shape2.IsNull());

    TopoDS_Edge edge1 = TopoDS::Edge(shape1);
    TopoDS_Edge edge2 = TopoDS::Edge(shape2);
    const BRepAdaptor_Curve curve1(edge1);
    const BRepAdaptor_Curve curve2(edge2);

    auto fnGetLineDirection = [](const Adaptor3d_Curve& curve) -> gp_Dir {
        throwErrorIf<ErrorCode::NotLinearEdge>(curve.GetType() != GeomAbs_Line);
        return curve.Line().Direction();
#if 0
        // Find the direction in case the curve is a pseudo line
        const GCPnts_QuasiUniformAbscissa pnts(curve, 64);
        throwErrorIf<ErrorCode::NotLinearEdge>(!pnts.IsDone());
        const gp_Pnt pntFirst = GeomUtils::d0(curve, pnts.Parameter(1));
        const gp_Pnt pntLast = GeomUtils::d0(curve, pnts.Parameter(pnts.NbPoints()));
        throwErrorIf<ErrorCode::NotLinearEdge>(pntFirst.Distance(pntLast) < Precision::Confusion());
        const gp_Dir dirLine(gp_Vec(pntFirst, pntLast));
        for (int i = 2; i <= pnts.NbPoints() - 1; ++i) {
            const gp_Pnt pntSample = GeomUtils::d0(curve, pnts.Parameter(i));
            throwErrorIf<ErrorCode::NotLinearEdge>(pntFirst.Distance(pntSample) < Precision::Confusion());
            const gp_Dir dirSample(gp_Vec(pntFirst, pntSample));
            throwErrorIf<ErrorCode::NotLinearEdge>(!dirLine.IsEqual(dirSample, Precision::Angular()));
        }
        return dirLine;
#endif
    };

    // Check edges are not parallel
    const gp_Dir dirLine1 = fnGetLineDirection(curve1);
    const gp_Dir dirLine2 = fnGetLineDirection(curve2);
    throwErrorIf<ErrorCode::ParallelEdges>(dirLine1.IsParallel(dirLine2, Precision::Angular()));

    // Let 'edge1' be the smallest entity regarding length
    if (GCPnts_AbscissaPoint::Length(curve1, 1e-6) > GCPnts_AbscissaPoint::Length(curve2, 1e-6))
        std::swap(edge1, edge2);

    // Move 'edge2' close to 'edge1' if needed
    BRepExtrema_DistShapeShape distEdges(edge1, edge2);
    throwErrorIf<ErrorCode::Unknown>(!distEdges.IsDone());
    const double minDist = distEdges.Value();
    if (minDist > Precision::Confusion()) {
        gp_Trsf trsf;
        trsf.SetTranslation(distEdges.PointOnShape2(1), distEdges.PointOnShape1(1));
        edge2 = TopoDS::Edge(BRepBuilderAPI_Transform(edge2, trsf, true/*copy*/));
    }

    // Compute angle by delegating to PrsDim_AngleDimension
    PrsDim_AngleDimension dimAngle(edge1, edge2);
    angleResult.pnt1 = dimAngle.FirstPoint();
    angleResult.pnt2 = dimAngle.SecondPoint();
    angleResult.pntCenter = dimAngle.CenterPoint();
    const gp_Vec vec1(angleResult.pntCenter, angleResult.pnt1);
    const gp_Vec vec2(angleResult.pntCenter, angleResult.pnt2);
    angleResult.value = vec1.Angle(vec2) * Quantity_Radian;

    return angleResult;
}

QuantityLength MeasureToolBRep::brepLength(const TopoDS_Shape& shape)
{
    throwErrorIf<ErrorCode::NotAllEdges>(shape.IsNull() || shape.ShapeType() != TopAbs_EDGE);
    const TopoDS_Edge& edge = TopoDS::Edge(shape);
    if (BRep_Tool::IsGeometric(edge)) {
        const BRepAdaptor_Curve curve(TopoDS::Edge(shape));
        const double len = GCPnts_AbscissaPoint::Length(curve, 1e-6);
        return len * Quantity_Millimeter;
    }
    else {
        TopLoc_Location loc;
        const Handle(Poly_Polygon3D)& polyline = BRep_Tool::Polygon3D(edge, loc);
        throwErrorIf<ErrorCode::NotGeometricOrPolygonEdge>(polyline.IsNull());
        double len = 0.;
        for (int i = 2; i <= polyline->NbNodes(); ++i) {
            const gp_Pnt& pnt1 = polyline->Nodes().Value(i - 1);
            const gp_Pnt& pnt2 = polyline->Nodes().Value(i);
            len += pnt1.Distance(pnt2);
        }

        return len * Quantity_Millimeter;
    }
}

} // namespace Mayo
