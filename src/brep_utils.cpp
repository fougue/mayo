#include "brep_utils.h"

namespace occ {

const char* BRepUtils::shapeTypeToString(TopAbs_ShapeEnum shapeType)
{
    switch (shapeType) {
    case TopAbs_COMPOUND: return "COMPOUND";
    case TopAbs_COMPSOLID: return "COMPSOLID";
    case TopAbs_SOLID: return "SOLID";
    case TopAbs_SHELL: return "SHELL";
    case TopAbs_FACE: return "FACE";
    case TopAbs_WIRE: return "WIRE";
    case TopAbs_EDGE: return "EDGE";
    case TopAbs_VERTEX: return "VERTEX";
    case TopAbs_SHAPE: return "SHAPE";
    }
    return "??";
}

} // namespace occ
