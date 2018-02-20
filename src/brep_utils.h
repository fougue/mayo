#pragma once

#include <TopAbs_ShapeEnum.hxx>

namespace occ {

class BRepUtils
{
public:
    static const char* shapeTypeToString(TopAbs_ShapeEnum shapeType);
};

} // namespace occ
