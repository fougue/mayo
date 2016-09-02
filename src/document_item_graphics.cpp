#include "document_item_graphics.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

DocumentItemGraphics::DocumentItemGraphics()
    : propertyMaterial(this, tr("Material"), &enum_Graphic3dNameOfMaterial()),
      propertyColor(this, tr("Color"))
{
}

} // namespace Mayo
