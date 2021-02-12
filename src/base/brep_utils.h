/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <string>

namespace Mayo {

struct BRepUtils {
    template<typename FUNC>
    static void forEachSubShape(TopExp_Explorer& explorer, FUNC fn);

    template<typename FUNC>
    static void forEachSubShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum shapeType, FUNC fn);

    template<typename FUNC>
    static void forEachSubFace(const TopoDS_Shape& shape, FUNC fn);

    static bool moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs);

    static int hashCode(const TopoDS_Shape& shape);

    static std::string shapeToString(const TopoDS_Shape& shape);
    static TopoDS_Shape shapeFromString(const std::string& str);
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
