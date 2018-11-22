/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "brep_utils.h"

#include <climits>

namespace Mayo {

bool BRepUtils::moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs)
{
    return lhs < rhs;
}

int BRepUtils::hashCode(const TopoDS_Shape& shape)
{
    return !shape.IsNull() ? shape.HashCode(INT_MAX) : -1;
}

} // namespace Mayo
