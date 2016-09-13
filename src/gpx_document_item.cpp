#include "gpx_document_item.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

GpxDocumentItem::GpxDocumentItem()
    : propertyMaterial(this, tr("Material"), &enum_Graphic3dNameOfMaterial()),
      propertyColor(this, tr("Color"))
{
}

} // namespace Mayo
