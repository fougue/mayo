/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_xde_document_item.h"

#include <cassert>

namespace Mayo {

GpxXdeDocumentItem::GpxXdeDocumentItem(XdeDocumentItem* item)
    : GpxCovariantDocumentItem(item),
      GpxBRepShapeCommonProperties(this)
{
    // TODO XCAFPrs_AISObject requires a root label containing a TopoDS_Shape
    // If the XDE document as many free top-level shapes then there is a problem
    // Doesn't work :
    //     XCAFDoc_DocumentTool::ShapesLabel()
    //     XCAFDoc_ShapeTool::BaseLabel()
    // The only viable solution(instead of creating a root shape label containing
    // the free top-level shapes) is to have an AIS object per free top-level
    // shape.
    const std::vector<TDF_Label> vecFreeShape = item->topLevelFreeShapes();
    assert(vecFreeShape.size() <= 1);
    if (!vecFreeShape.empty()) {
        m_hndGpxObject = new XCAFPrs_AISObject(vecFreeShape.front());
        GpxDocumentItem::initForGpxBRepShape(m_hndGpxObject);
        GpxBRepShapeCommonProperties::initCommonProperties(this, m_hndGpxObject);
    }
    else { // Dummy
        m_hndGpxObject = new XCAFPrs_AISObject(item->cafDoc()->Main());
    }
}

void GpxXdeDocumentItem::onPropertyChanged(Property* prop)
{
    Handle_AIS_InteractiveObject hndGpx = this->handleGpxObject();
    if (prop == &this->propertyMaterial) {
        GpxBRepShapeCommonProperties::handlePropertyMaterial(
                    &this->propertyMaterial, hndGpx);
    }
    else if (prop == &this->propertyColor) {
        GpxBRepShapeCommonProperties::handlePropertyColor(
                    &this->propertyColor, hndGpx);
    }
    GpxBRepShapeCommonProperties::handleCommonPropertyChange(prop, hndGpx);
    GpxDocumentItem::onPropertyChanged(prop);
}

} // namespace Mayo
