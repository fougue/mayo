/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_brep_mesh_parameters.h"
#include "occ_handle.h"

#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <string>

namespace Mayo {

class TaskProgress;

// Provides helper functions for OpenCascade TKBRep library
struct BRepUtils {
    // Creates a valid and empty TopoDS_Compound shape
    static TopoDS_Compound makeEmptyCompound();

    // Adds 'shape' in target shape 'ptrTargetShape'
    static void addShape(TopoDS_Shape* ptrTargetShape, const TopoDS_Shape& shape);

    // Creates a non-geometric TopoDS_Edge wrapping 'polygon'
    static TopoDS_Edge makeEdge(const OccHandle<Poly_Polygon3D>& polygon);

    // Creates a non-geometric TopoDS_Face wrapping triangulation 'mesh'
    static TopoDS_Face makeFace(const OccHandle<Poly_Triangulation>& mesh);

    // Iterates with 'explorer' and executes 'fn' for each sub-shape
    template<typename Function>
    static void forEachSubShape(TopExp_Explorer& explorer, Function fn);

    // Explores 'shape' and executes 'fn' for each sub-shape of type 'shapeType'
    template<typename Function>
    static void forEachSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum shapeType, Function fn);

    // Explores 'shape' and executes 'fn' for each sub-face
    template<typename Function>
    static void forEachSubFace(const TopoDS_Shape& shape, Function fn);

    // Is shape type 'lhs' more complex than 'rhs'?
    // Complexity here is the degree of abstraction provided(eg face type is more complex than edge type)
    static bool moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs);

    // Returns hash code computed from 'shape'
    // Computation uses the internal TShape and Location, but Orientation is not considered
    // Returned hash code is in the range [1, max(int)]
    static size_t hashCode(const TopoDS_Shape& shape);

    // Serializes 'shape' into a string representation
    static std::string shapeToString(const TopoDS_Shape& shape);

    // Deserializes string 'str' obtained from 'shapeToToString()' into a shape object
    static TopoDS_Shape shapeFromString(const std::string& str);

    // Does 'edge' rely on 3D curve or curve on surface?
    static bool isGeometric(const TopoDS_Edge& edge);

    // Does 'face' rely on a geometric surface?
    static bool isGeometric(const TopoDS_Face& face);

    // Computes a mesh representation of 'shape' using OpenCascade meshing algorithm
    static void computeMesh(
            const TopoDS_Shape& shape,
            const OccBRepMeshParameters& params,
            TaskProgress* progress = nullptr
    );
};



// --
// -- Implementation
// --

template<typename Function>
void BRepUtils::forEachSubShape(TopExp_Explorer& explorer, Function fn)
{
    while (explorer.More()) {
        fn(explorer.Current());
        explorer.Next();
    }
}

template<typename Function>
void BRepUtils::forEachSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum shapeType, Function fn)
{
    TopExp_Explorer expl(shape, shapeType);
    BRepUtils::forEachSubShape(expl, std::move(fn));
}

template<typename Function>
void BRepUtils::forEachSubFace(const TopoDS_Shape& shape, Function fn)
{
    for (TopExp_Explorer expl(shape, TopAbs_FACE); expl.More(); expl.Next())
        fn(TopoDS::Face(expl.Current()));
}

} // namespace Mayo
