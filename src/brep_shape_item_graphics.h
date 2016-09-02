#pragma once

#include "brep_shape_item.h"
#include "document_item_graphics.h"
#include <AIS_Shape.hxx>

namespace Mayo {

class BRepShapeItemGraphics :
        public CovariantDocumentItemGraphics<BRepShapeItem, AIS_Shape, Handle_AIS_Shape>
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::BRepShapeItemGraphics)

public:
    BRepShapeItemGraphics(BRepShapeItem* item);

    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowFaceBoundary;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
};

} // namespace Mayo
