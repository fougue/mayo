/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "measure_tool_brep.h"

#include "../base/brep_utils.h"
#include "../base/geom_utils.h"
#include "../base/math_utils.h"
#include "../base/mesh_utils.h"
#include "../base/occ_handle.h"
#include "../base/text_id.h"
#include "../graphics/graphics_shape_object_driver.h"

#include <AIS_Shape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_OBB.hxx>
#include <ElSLib.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_QuasiUniformAbscissa.hxx>
#include <GC_MakeCircle.hxx>
#include <GProp_GProps.hxx>
#include <Precision.hxx>
#include <ProjLib.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Elips.hxx>
#include <math_Jacobi.hxx>

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070500
#  include <PrsDim_AngleDimension.hxx>
#else
#  include <AIS_AngleDimension.hxx>
using PrsDim_AngleDimension = AIS_AngleDimension;
#endif

#include <algorithm>
#include <array>
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
    NotGeometricOrTriangulationFace,
    MinDistanceFailure,
    CenterFailure,
    NotAllEdges,
    NotLinearEdge,
    NotAllFaces,
    ParallelEdges,
    BoundingBoxIsVoid
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
        case ErrorCode::NotGeometricOrTriangulationFace:
            return textIdTr("Entity must be a geometric or triangulation face");
        case ErrorCode::MinDistanceFailure:
            return textIdTr("Computation of minimum distance failed");
        case ErrorCode::CenterFailure:
            return textIdTr("Unable to find center of the shape");
        case ErrorCode::NotAllEdges:
            return textIdTr("All entities must be edges");
        case ErrorCode::NotLinearEdge:
            return textIdTr("Entity must be a linear edge");
        case ErrorCode::NotAllFaces:
            return textIdTr("All entities must be faces");
        case ErrorCode::ParallelEdges:
            return textIdTr("Entities must not be parallel");
        case ErrorCode::BoundingBoxIsVoid:
            return textIdTr("Bounding box computed is void");
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
    auto brepOwner = OccHandle<StdSelect_BRepOwner>::DownCast(owner);
    TopLoc_Location ownerLoc = owner->Location();
#if OCC_VERSION_HEX >= 0x070600
    // Force scale factor to 1
    // If scale factor <> 1 then it will cause a crash(exception) in TopoDS_Shape::Move() starting
    // from OpenCascade >= 7.6
    const double absScale = std::abs(ownerLoc.Transformation().ScaleFactor());
    const double scalePrec = TopLoc_Location::ScalePrec();
    if (absScale < (1. - scalePrec) || absScale > (1. + scalePrec)) {
        gp_Trsf trsf = ownerLoc.Transformation();
        trsf.SetScaleFactor(1.);
        ownerLoc = trsf;
    }
#endif
    return brepOwner ? brepOwner->Shape().Moved(ownerLoc) : nullShape;
}

gp_Pnt computeShapeCenter(const TopoDS_Shape& shape)
{
    const TopAbs_ShapeEnum shapeType = shape.ShapeType();

    if (shapeType == TopAbs_VERTEX)
        return BRep_Tool::Pnt(TopoDS::Vertex(shape));

    GProp_GProps shapeProps;
    if (shapeType == TopAbs_FACE) {
        // TODO Consider case where the face is cylindrical
        BRepGProp::SurfaceProperties(shape, shapeProps);
    }
    else if (shapeType == TopAbs_WIRE) {
        BRepGProp::LinearProperties(shape, shapeProps);
    }
    else if (shapeType == TopAbs_EDGE) {
        try {
            const MeasureCircle circle = MeasureToolBRep::brepCircle(shape);
            return circle.value.Location();
        }
        catch (const IMeasureError&) {
            BRepGProp::LinearProperties(shape, shapeProps);
        }
    }
    else {
        throw BRepMeasureError<ErrorCode::CenterFailure>();
    }

    throwErrorIf<ErrorCode::CenterFailure>(shapeProps.Mass() < Precision::Confusion());
    return shapeProps.CentreOfMass();
}

// Defines the result of fittedPlane() function
struct FittedPlaneResult {
    bool success = false;
    gp_Pln value;
    double coplanarity = Precision::Infinite();
};

// Computes optimal plane from input 3D points
FittedPlaneResult fittedPlane(Span<const gp_Pnt> points)
{
    // Compute centroid point
    gp_Pnt centroid;
    for (const gp_Pnt& pnt : points)
        centroid.ChangeCoord() += pnt.Coord();

    centroid.ChangeCoord() /= int(points.size());

    // Compute covariance matrix of centered coordinates
    math_Matrix covMatrix(1, 3, 1, 3, 0.);
    for (const gp_Pnt& pnt : points) {
        const gp_XYZ v = pnt.XYZ() - centroid.XYZ();
        for (int i = 1; i <= 3; ++i) {
            for (int j = 1; j <= 3; ++j)
                covMatrix(i, j) += v.Coord(i) * v.Coord(j);
        }
    }

    // Find eigen vectors and values of the covariance matrix
    math_Jacobi jacobiSolver(covMatrix);
    if (!jacobiSolver.IsDone())
        return {};

    const math_Vector& eigenValues = jacobiSolver.Values();
    const math_Matrix& eigenVectors = jacobiSolver.Vectors();

    int countOfNonNullEigenValues = 0;
    for (int i = 1; i <= 3; ++i) {
        if (!MathUtils::fuzzyIsNull(eigenValues(i)))
            ++countOfNonNullEigenValues;
    }

    if (countOfNonNullEigenValues == 0 || countOfNonNullEigenValues == 1)
        return {};

    const int eigenMinIndex = eigenValues.Min();
    const int eigenMaxIndex = eigenValues.Max();

    // Plane normal is the eigen vector associated to smallest eigen value
    gp_Vec normal(
        eigenVectors(1, eigenMinIndex),
        eigenVectors(2, eigenMinIndex),
        eigenVectors(3, eigenMinIndex)
    );
    if (normal.IsEqual(gp_Vec{}, Precision::Confusion(), Precision::Angular()))
        return {};

    FittedPlaneResult result;
    result.success = true;
    result.value = gp_Pln{centroid, normal.Normalized()};
    if (!MathUtils::fuzzyIsNull(eigenValues(eigenMaxIndex)))
        result.coplanarity = eigenValues(eigenMinIndex) / eigenValues(eigenMaxIndex);
    else
        result.coplanarity = 0.;

    return result;
}

// Defines the result of fittedCircle_taubin() function
struct FittedCircle2DResult {
    bool success = false;
    gp_Pnt2d center;
    double radius = 0.;
};

// Compute optimal circle from input 3D points and plane
// This function uses Taubin method that is considered one of the best for circular regression(more
// stable than Kasa method)
FittedCircle2DResult fittedCircle2D_taubin(Span<const gp_Pnt2d> points)
{
    const size_t N = points.size();

    // Center input points
    double meanX = 0;
    double meanY = 0;
    for (const gp_Pnt2d& pnt : points) {
        meanX += pnt.X();
        meanY += pnt.Y();
    }

    meanX /= N;
    meanY /= N;

    // Build needed matrix
    double Sxx = 0, Syy = 0, Sxy = 0;
    double Sxz = 0, Syz = 0, Szz = 0;
    for (const gp_Pnt2d& pnt : points) {
        const double x = pnt.X() - meanX;
        const double y = pnt.Y() - meanY;
        const double z = x*x + y*y;
        Sxx += x * x;
        Syy += y * y;
        Sxy += x * y;
        Sxz += x * z;
        Syz += y * z;
        Szz += z * z;
    }

    // Solve quadratic system
    const double Cov_xy = Sxx * Syy - Sxy * Sxy;
    // If Cov_xy is null this means input points are aligned
    if (MathUtils::fuzzyIsNull(Cov_xy))
        return {};

    const double A2 = 0.5 * (Sxz * Syy - Syz * Sxy) / Cov_xy;
    const double B2 = 0.5 * (Syz * Sxx - Sxz * Sxy) / Cov_xy;

    const double centerX = A2 + meanX;
    const double centerY = B2 + meanY;

    double radius = 0;
    for (const gp_Pnt2d& pnt : points) {
        const double dx = pnt.X() - centerX;
        const double dy = pnt.Y() - centerY;
        radius += std::sqrt(dx*dx + dy*dy);
    }

    radius /= N;
    FittedCircle2DResult result;
    result.success = true;
    result.center = gp_Pnt2d{centerX, centerY};
    result.radius = radius;
    return result;
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
    case MeasureType::MinDistance:
    case MeasureType::CenterDistance: {
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
    case MeasureType::BoundingBox: {
        static const GraphicsObjectSelectionMode modes[] = {
            //AIS_Shape::SelectionMode(TopAbs_FACE),
            //AIS_Shape::SelectionMode(TopAbs_SOLID)
            AIS_Shape::SelectionMode(TopAbs_COMPOUND)
        };
        return modes;
    }
    default: {
        return {};
    }
    } // endswitch
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

MeasureDistance MeasureToolBRep::minDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const
{
    return brepMinDistance(getShape(owner1), getShape(owner2));
}

MeasureDistance MeasureToolBRep::centerDistance(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const
{
    return brepCenterDistance(getShape(owner1), getShape(owner2));
}

MeasureAngle MeasureToolBRep::angle(const GraphicsOwnerPtr& owner1, const GraphicsOwnerPtr& owner2) const
{
    return brepAngle(getShape(owner1), getShape(owner2));
}

MeasureLength MeasureToolBRep::length(const GraphicsOwnerPtr& owner) const
{
    return brepLength(getShape(owner));
}

MeasureArea MeasureToolBRep::area(const GraphicsOwnerPtr& owner) const
{
    return brepArea(getShape(owner));
}

MeasureBoundingBox MeasureToolBRep::boundingBox(const GraphicsOwnerPtr& owner) const
{
    return brepBoundingBox(getShape(owner));
}

gp_Pnt MeasureToolBRep::brepVertexPosition(const TopoDS_Shape& shape)
{
    throwErrorIf<ErrorCode::NotVertex>(shape.IsNull() || shape.ShapeType() != TopAbs_VERTEX);
    return BRep_Tool::Pnt(TopoDS::Vertex(shape));
}

MeasureCircle MeasureToolBRep::brepCircleFromGeometricEdge(const TopoDS_Edge& edge)
{
    const BRepAdaptor_Curve curve(edge);
    std::optional<gp_Circ> circle;
    if (curve.GetType() == GeomAbs_Circle) {
        circle = curve.Circle();
    }
    else if (curve.GetType() == GeomAbs_Ellipse) {
        const gp_Elips ellipse = curve.Ellipse();
        if (std::abs(ellipse.MinorRadius() - ellipse.MajorRadius()) < Precision::Confusion())
            circle = gp_Circ{ ellipse.Position(), ellipse.MinorRadius() };
    }
    else {
        // Discretize input curve
        constexpr size_t DiscreteCount = 64;
        std::array<gp_Pnt, DiscreteCount> discrete;
        {
            const GCPnts_QuasiUniformAbscissa pnts(curve, int(discrete.size()));
            throwErrorIf<ErrorCode::NotCircularEdge>(!pnts.IsDone());
            for (int i = 1; i <= pnts.NbPoints(); ++i)
                discrete[i - 1] = GeomUtils::d0(curve, pnts.Parameter(i));
        }

        // Compute plane containing the points
        const FittedPlaneResult plane = fittedPlane(discrete);
        throwErrorIf<ErrorCode::NotCircularEdge>(!plane.success);
        throwErrorIf<ErrorCode::NotCircularEdge>(plane.coplanarity > 1e-4);

        // Project points on the plane
        std::array<gp_Pnt2d, DiscreteCount> discrete2d;
        for (size_t i = 0; i < DiscreteCount; ++i)
            discrete2d[i] = ProjLib::Project(plane.value, discrete[i]);

        // Compute optimal circle from 2D points
        const FittedCircle2DResult circle2d = fittedCircle2D_taubin(discrete2d);
        throwErrorIf<ErrorCode::NotCircularEdge>(!circle2d.success);

        // Check mean(average) relative error
        double sumError = 0.;
        for (const gp_Pnt2d& pnt : discrete2d) {
            const double distCenter = pnt.Distance(circle2d.center);
            sumError += std::abs(distCenter - circle2d.radius);
        }

        const double meanError = sumError / circle2d.radius;
        throwErrorIf<ErrorCode::NotCircularEdge>(meanError > 0.01); // error must be <= 1%

        // Project the 2D center in 3D space to get the circle result
        const gp_Pnt2d& center2d = circle2d.center;
        const gp_Pnt center = ElSLib::Value(center2d.X(), center2d.Y(), plane.value);
        circle = gp_Circ{gp_Ax2{center, plane.value.Axis().Direction()}, circle2d.radius};
#if 0
        // Try to create a circle from 3 sample points on the curve
        {
            const GCPnts_QuasiUniformAbscissa pnts(curve, 4); // More points to avoid confusion
            throwErrorIf<ErrorCode::NotCircularEdge>(!pnts.IsDone() || pnts.NbPoints() < 3);
            const GC_MakeCircle makeCirc(
                GeomUtils::d0(curve, pnts.Parameter(1)),
                GeomUtils::d0(curve, pnts.Parameter(2)),
                GeomUtils::d0(curve, pnts.Parameter(3))
            );
            throwErrorIf<ErrorCode::NotCircularEdge>(!makeCirc.IsDone());
            circle = makeCirc.Value()->Circ();
        }

        // Take more sample points on the curve and check for each that:
        //     dist(pntSample, pntCircleCenter) - circleRadius < tolerance
        {
            const GCPnts_QuasiUniformAbscissa pnts(curve, 64);
            throwErrorIf<ErrorCode::NotCircularEdge>(!pnts.IsDone());
            for (int i = 1; i <= pnts.NbPoints(); ++i) {
                const gp_Pnt pntSample = GeomUtils::d0(curve, pnts.Parameter(i));
                const double dist = pntSample.Distance(circle->Location());
                throwErrorIf<ErrorCode::NotCircularEdge>(std::abs(dist - circle->Radius()) > 1e-4);
            }
        }
#endif
    }

    throwErrorIf<ErrorCode::NotCircularEdge>(!circle);
    MeasureCircle result;
    result.pntAnchor = GeomUtils::d0(curve, curve.FirstParameter());
    result.isArc = !curve.IsClosed();
    result.value = circle.value();
    return result;
}

MeasureCircle MeasureToolBRep::brepCircleFromPolygonEdge(const TopoDS_Edge& edge)
{
    TopLoc_Location loc;
    const OccHandle<Poly_Polygon3D>& polyline = BRep_Tool::Polygon3D(edge, loc);
    throwErrorIf<ErrorCode::NotGeometricOrPolygonEdge>(polyline.IsNull() || polyline->NbNodes() < 7);
    // Try to create a circle from 3 sample points
    const GC_MakeCircle makeCirc(
        polyline->Nodes().First(),
        polyline->Nodes().Value(1 + polyline->NbNodes() / 3),
        polyline->Nodes().Value(1 + 2 * polyline->NbNodes() / 3)
    );
    throwErrorIf<ErrorCode::NotCircularEdge>(!makeCirc.IsDone());
    const gp_Circ circle = makeCirc.Value()->Circ();

    // Check for all points that:
    //     dist(pnt, pntCircleCenter) - circleRadius < tolerance
    const double tol = !MathUtils::fuzzyIsNull(polyline->Deflection()) ? 1.5 * polyline->Deflection() : 1e-4;
    for (const gp_Pnt& pnt : polyline->Nodes()) {
        const double dist = pnt.Distance(circle.Location());
        throwErrorIf<ErrorCode::NotCircularEdge>(std::abs(dist - circle.Radius()) > tol);
    }

    MeasureCircle result;
    result.pntAnchor = polyline->Nodes().First().Transformed(loc);
    result.isArc = !polyline->Nodes().First().IsEqual(polyline->Nodes().Last(), Precision::Confusion());
    result.value = circle;
    return result;
}

MeasureCircle MeasureToolBRep::brepCircle(const TopoDS_Shape& shape)
{
    throwErrorIf<ErrorCode::NotCircularEdge>(shape.IsNull() || shape.ShapeType() != TopAbs_EDGE);
    const TopoDS_Edge& edge = TopoDS::Edge(shape);
    if (BRepUtils::isGeometric(edge))
        return MeasureToolBRep::brepCircleFromGeometricEdge(edge);
    else
        return MeasureToolBRep::brepCircleFromPolygonEdge(edge);
}

MeasureDistance MeasureToolBRep::brepMinDistance(
        const TopoDS_Shape& shape1, const TopoDS_Shape& shape2
    )
{
    throwErrorIf<ErrorCode::NotBRepShape>(shape1.IsNull());
    throwErrorIf<ErrorCode::NotBRepShape>(shape2.IsNull());

    BRepExtrema_DistShapeShape dist;
    try {
        dist.LoadS1(shape1);
        dist.LoadS2(shape2);
        dist.Perform();
    } catch (...) {
        throw BRepMeasureError<ErrorCode::MinDistanceFailure>();
    }

    throwErrorIf<ErrorCode::MinDistanceFailure>(!dist.IsDone());

    MeasureDistance distResult;
    distResult.pnt1 = dist.PointOnShape1(1);
    distResult.pnt2 = dist.PointOnShape2(1);
    distResult.value = dist.Value() * Quantity_Millimeter;
    distResult.type = MeasureDistance::Type::Mininmum;
    return distResult;
}

MeasureDistance MeasureToolBRep::brepCenterDistance(
        const TopoDS_Shape& shape1, const TopoDS_Shape& shape2
    )
{
    throwErrorIf<ErrorCode::NotBRepShape>(shape1.IsNull());
    throwErrorIf<ErrorCode::NotBRepShape>(shape2.IsNull());
    
    const gp_Pnt centerOfMass1 = computeShapeCenter(shape1);
    const gp_Pnt centerOfMass2 = computeShapeCenter(shape2);

    MeasureDistance distResult;
    distResult.pnt1 = centerOfMass1;
    distResult.pnt2 = centerOfMass2;
    distResult.value = centerOfMass1.Distance(centerOfMass2) * Quantity_Millimeter;
    distResult.type = MeasureDistance::Type::CenterToCenter;
    return distResult;
}

MeasureAngle MeasureToolBRep::brepAngle(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2)
{
    MeasureAngle angleResult;

    throwErrorIf<ErrorCode::NotBRepShape>(shape1.IsNull());
    throwErrorIf<ErrorCode::NotBRepShape>(shape2.IsNull());

    TopoDS_Edge edge1 = TopoDS::Edge(shape1);
    TopoDS_Edge edge2 = TopoDS::Edge(shape2);
    // TODO What if edge1 and edge2 are not geometric?
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

MeasureLength MeasureToolBRep::brepLength(const TopoDS_Shape& shape)
{
    MeasureLength lenResult;

    throwErrorIf<ErrorCode::NotAllEdges>(shape.IsNull() || shape.ShapeType() != TopAbs_EDGE);
    const TopoDS_Edge& edge = TopoDS::Edge(shape);
    if (BRepUtils::isGeometric(edge)) {
        const BRepAdaptor_Curve curve(edge);
        const double len = GCPnts_AbscissaPoint::Length(curve, 1e-6);
        lenResult.value = len * Quantity_Millimeter;
        const GCPnts_QuasiUniformAbscissa pnts(curve, 3);
        if (pnts.IsDone() && pnts.NbPoints() == 3) {
            lenResult.middlePnt = GeomUtils::d0(curve, pnts.Parameter(2));
        }
        else {
            const double midParam = (curve.FirstParameter() + curve.LastParameter()) / 2.;
            lenResult.middlePnt = GeomUtils::d0(curve, midParam);
        }
    }
    else {
        TopLoc_Location loc;
        const OccHandle<Poly_Polygon3D>& polyline = BRep_Tool::Polygon3D(edge, loc);
        throwErrorIf<ErrorCode::NotGeometricOrPolygonEdge>(polyline.IsNull());
        double len = 0.;
        // Compute length of the polygon
        for (int i = 2; i <= polyline->NbNodes(); ++i) {
            const gp_Pnt& pnt1 = polyline->Nodes().Value(i - 1);
            const gp_Pnt& pnt2 = polyline->Nodes().Value(i);
            len += pnt1.Distance(pnt2);
        }

        lenResult.value = len * Quantity_Millimeter;

        // Compute middle point of the polygon
        double accumLen = 0.;
        for (int i = 2; i <= polyline->NbNodes() && accumLen < (len / 2.); ++i) {
            const gp_Pnt& pnt1 = polyline->Nodes().Value(i - 1);
            const gp_Pnt& pnt2 = polyline->Nodes().Value(i);
            accumLen += pnt1.Distance(pnt2);
            if (accumLen > (len / 2.)) {
                const gp_Pnt pntLoc1 = pnt1.Transformed(loc);
                const gp_Pnt pntLoc2 = pnt2.Transformed(loc);
                lenResult.middlePnt = pntLoc1.Translated(gp_Vec{pntLoc1, pntLoc2} / 2.);
            }
        }
    }

    return lenResult;
}

MeasureArea MeasureToolBRep::brepArea(const TopoDS_Shape& shape)
{
    MeasureArea areaResult;
    throwErrorIf<ErrorCode::NotAllFaces>(shape.IsNull() || shape.ShapeType() != TopAbs_FACE);
    const TopoDS_Face& face = TopoDS::Face(shape);

    if (BRepUtils::isGeometric(face)) {
        GProp_GProps gprops;
        BRepGProp::SurfaceProperties(face, gprops);
        const double area = gprops.Mass();
        areaResult.value = area * Quantity_SquareMillimeter;

        const BRepAdaptor_Surface surface(face);
        areaResult.middlePnt = surface.Value(
            (surface.FirstUParameter() + surface.LastUParameter()) / 2.,
            (surface.FirstVParameter() + surface.LastVParameter()) / 2.
        );
    }
    else {
        TopLoc_Location loc;
        const OccHandle<Poly_Triangulation>& triangulation = BRep_Tool::Triangulation(face, loc);
        throwErrorIf<ErrorCode::NotGeometricOrTriangulationFace>(triangulation.IsNull());
        areaResult.value = MeshUtils::triangulationArea(triangulation) * Quantity_SquareMillimeter;

        for (int i = 1; i <= triangulation->NbNodes(); ++i) {
            const gp_Pnt node = triangulation->Node(i).Transformed(loc);
            areaResult.middlePnt.Translate(node.XYZ());
        }

        areaResult.middlePnt.ChangeCoord().Divide(triangulation->NbNodes());
    }

    return areaResult;
}

MeasureBoundingBox MeasureToolBRep::brepBoundingBox(const TopoDS_Shape& shape)
{
    MeasureBoundingBox measure;
#if 0
    Bnd_OBB bnd;
    BRepBndLib::AddOBB(shape, bnd);
    //BRepBndLib::AddOBB(shape, bnd, false/*!useTriangulation*/, true/*optimal*/, false/*!useShapeTolerance*/);

    gp_Pnt points[8] = {};
    bnd.GetVertex(points);
    measure.cornerMin = points[0];
    measure.cornerMax = points[7];
    //measure.isAxisAligned = bnd.IsAABox();
#else
    Bnd_Box bnd;
    BRepBndLib::AddOptimal(shape, bnd);
    throwErrorIf<ErrorCode::BoundingBoxIsVoid>(bnd.IsVoid());
    measure.cornerMin = bnd.CornerMin();
    measure.cornerMax = bnd.CornerMax();
    measure.xLength = std::abs(measure.cornerMax.X() - measure.cornerMin.X()) * Quantity_Millimeter;
    measure.yLength = std::abs(measure.cornerMax.Y() - measure.cornerMin.Y()) * Quantity_Millimeter;
    measure.zLength = std::abs(measure.cornerMax.Z() - measure.cornerMin.Z()) * Quantity_Millimeter;
    measure.volume = measure.xLength * measure.yLength * measure.zLength;
#endif
    return measure;
}

} // namespace Mayo
