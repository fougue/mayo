/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "brep_utils.h"

namespace Mayo {

bool BRepUtils::moreComplex(TopAbs_ShapeEnum lhs, TopAbs_ShapeEnum rhs) {
    return lhs < rhs;
}

} // namespace Mayo
