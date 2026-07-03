/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "brep_utils.h"

#include "tkernel_utils.h"
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
#  include "occ_progress_indicator.h"
#endif

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <TopoDS_Compound.hxx>
#include <climits>
#include <sstream>

namespace Mayo {

TopoDS_Compound BRepUtils::makeEmptyCompound()
{
    TopoDS_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);
    return comp;
}

void BRepUtils::addShape(TopoDS_Shape* ptrTargetShape, const TopoDS_Shape& shape)
{
    if (!ptrTargetShape)
        return;

    if (shape.IsNull())
        return;

    if (ptrTargetShape->IsNull())
        *ptrTargetShape = BRepUtils::makeEmptyCompound();

    TopoDS_Builder builder;
    builder.Add(*ptrTargetShape, shape);
}

TopoDS_Edge BRepUtils::makeEdge(const OccHandle<Poly_Polygon3D>& polygon)
{
    TopoDS_Edge edge;
    BRep_Builder builder;
    builder.MakeEdge(edge);
    builder.UpdateEdge(edge, polygon);
    return edge;
}

TopoDS_Face BRepUtils::makeFace(const OccHandle<Poly_Triangulation>& mesh)
{
    TopoDS_Face face;
    BRep_Builder builder;
    builder.MakeFace(face);
    builder.UpdateFace(face, mesh);
    return face;
}

bool BRepUtils::moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs)
{
    return lhs < rhs;
}

size_t BRepUtils::hashCode(const TopoDS_Shape& shape)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 8, 0)
    return std::hash<TopoDS_Shape>{}(shape);
#else
    return shape.HashCode(INT_MAX);
#endif
}

std::string BRepUtils::shapeToString(const TopoDS_Shape& shape)
{
    std::ostringstream oss(std::ios_base::out);
    BRepTools::Write(shape, oss);
    return oss.str();
}

TopoDS_Shape BRepUtils::shapeFromString(const std::string& str)
{
    TopoDS_Shape shape;
    BRep_Builder brepBuilder;
    std::istringstream iss(str, std::ios_base::in);
    BRepTools::Read(shape, iss, brepBuilder);
    return shape;
}

bool Mayo::BRepUtils::isGeometric(const TopoDS_Edge &edge)
{
    return BRep_Tool::IsGeometric(edge);
}

bool BRepUtils::isGeometric(const TopoDS_Face& face)
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    return BRep_Tool::IsGeometric(face);
#else
    auto tface = static_cast<const BRep_TFace*>(face.TShape().get());
    return !tface->Surface().IsNull();
#endif
}

void BRepUtils::computeMesh(
        const TopoDS_Shape& shape,
        const OccBRepMeshParameters& params,
        [[maybe_unused]]TaskProgress* progress
    )
{
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    auto indicator = makeOccHandle<OccProgressIndicator>(progress);
    [[maybe_unused]]BRepMesh_IncrementalMesh mesher(shape, params, TKernelUtils::start(indicator));
#else
    [[maybe_unused]]BRepMesh_IncrementalMesh mesher(shape, params);
#endif
}

} // namespace Mayo
