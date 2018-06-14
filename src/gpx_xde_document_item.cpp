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
    const TDF_LabelSequence seqShapeLabel = item->topLevelFreeShapeLabels();
    assert(seqShapeLabel.Size() <= 1);
    if (!seqShapeLabel.IsEmpty()) {
        m_hndGpxObject = new XCAFPrs_AISObject(seqShapeLabel.First());
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
