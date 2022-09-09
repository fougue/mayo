/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_brep_mesh_parameters.h"

#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <string>

namespace Mayo {

class TaskProgress;

// Provides helper functions for OpenCascade TKBRep library
struct BRepUtils {
    // Iterates with 'explorer' and executes 'fn' for each sub-shape
    template<typename FUNC>
    static void forEachSubShape(TopExp_Explorer& explorer, FUNC fn);

    // Explores 'shape' and executes 'fn' for each sub-shape of type 'shapeType'
    template<typename FUNC>
    static void forEachSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum shapeType, FUNC fn);

    // Explores 'shape' and executes 'fn' for each sub-face
    template<typename FUNC>
    static void forEachSubFace(const TopoDS_Shape& shape, FUNC fn);

    // Is shape type 'lhs' more complex than 'rhs'?
    // Complexity here is the degree of abstraction provided(eg face type is more complex than edge type)
    static bool moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs);

    // Returns hash code computed from 'shape'
    // Computation uses the internal TShape and Location, but Orientation is not considered
    // Returned hash code is in the range [1, max(int)]
    static int hashCode(const TopoDS_Shape& shape);

    // Serializes 'shape' into a string representation
    static std::string shapeToString(const TopoDS_Shape& shape);

    // Deserializes string 'str' obtained from 'shapeToToString()' into a shape object
    static TopoDS_Shape shapeFromString(const std::string& str);

    // Computes a mesh representation of 'shape' using OpenCascade meshing algorithm
    static void computeMesh(
            const TopoDS_Shape& shape,
            const OccBRepMeshParameters& params,
            TaskProgress* progress = nullptr);
};



// --
// -- Implementation
// --

template<typename FUNC>
void BRepUtils::forEachSubShape(TopExp_Explorer& explorer, FUNC fn)
{
    while (explorer.More()) {
        fn(explorer.Current());
        explorer.Next();
    }
}

template<typename FUNC>
void BRepUtils::forEachSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum shapeType, FUNC fn)
{
    TopExp_Explorer expl(shape, shapeType);
    BRepUtils::forEachSubShape(expl, std::move(fn));
}

template<typename FUNC>
void BRepUtils::forEachSubFace(const TopoDS_Shape& shape, FUNC fn)
{
    for (TopExp_Explorer expl(shape, TopAbs_FACE); expl.More(); expl.Next())
        fn(TopoDS::Face(expl.Current()));
}

} // namespace Mayo
