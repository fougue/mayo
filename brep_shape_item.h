#pragma once

#include "document_item.h"
#include <TopoDS_Shape.hxx>

namespace Mayo {

class BRepShapeItem : public PartItem
{
public:
    const TopoDS_Shape& brepShape() const;
    void setBRepShape(const TopoDS_Shape& shape);

    bool isNull() const override;

    static const char* type;
    const char* dynType() const override;

private:
    TopoDS_Shape m_brepShape;
};

} // namespace Mayo
