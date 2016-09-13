#pragma once

#include "brep_shape_item.h"
#include "gpx_document_item.h"
#include <AIS_Shape.hxx>

namespace Mayo {

class GpxBRepShapeItem :
        public GpxCovariantDocumentItem<BRepShapeItem, AIS_Shape, Handle_AIS_Shape>
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxBRepShapeItem)

public:
    GpxBRepShapeItem(BRepShapeItem* item);

    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowFaceBoundary;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
};

} // namespace Mayo
