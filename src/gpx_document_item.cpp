#include "gpx_document_item.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <QtCore/QCoreApplication>

namespace Mayo {

GpxDocumentItem::GpxDocumentItem()
    : propertyIsVisible(this, tr("Visible?")),
      propertyMaterial(this, tr("Material"), &enum_Graphic3dNameOfMaterial()),
      propertyColor(this, tr("Color"))
{
    Mayo_PropertyChangedBlocker(this);
    this->propertyIsVisible.setValue(true);
}

void GpxDocumentItem::onPropertyChanged(Property *prop)
{
    if (prop == &this->propertyIsVisible) {
        Handle_AIS_InteractiveContext cxt = this->handleGpxObject()->GetContext();
        if (this->propertyIsVisible.value())
            cxt->Display(this->handleGpxObject());
        else
            cxt->Erase(this->handleGpxObject());
    }
}

} // namespace Mayo
