/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "brep_utils.h"

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <climits>
#include <sstream>

namespace Mayo {

bool BRepUtils::moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs)
{
    return lhs < rhs;
}

int BRepUtils::hashCode(const TopoDS_Shape& shape)
{
    return !shape.IsNull() ? shape.HashCode(INT_MAX) : -1;
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

} // namespace Mayo
