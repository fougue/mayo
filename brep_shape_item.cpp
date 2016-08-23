#include "brep_shape_item.h"

namespace Mayo {

const TopoDS_Shape &BRepShapeItem::brepShape() const
{
    return m_brepShape;
}

void BRepShapeItem::setBRepShape(const TopoDS_Shape &shape)
{
    m_brepShape = shape;
}

bool BRepShapeItem::isNull() const
{
    return m_brepShape.IsNull();
}

const char* BRepShapeItem::type = "8095882e-8b1f-4b19-a0dd-f74f27748169";
const char* BRepShapeItem::dynType() const { return BRepShapeItem::type; }

} // namespace Mayo
