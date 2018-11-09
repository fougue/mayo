/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "xde_shape_property_owner.h"
#include "string_utils.h"

namespace Mayo {

XdeShapePropertyOwner::XdeShapePropertyOwner(
        const XdeDocumentItem *docItem,
        const TDF_Label &label,
        XdeDocumentItem::ShapePropertiesOption opt)
    : m_propertyName(this, tr("Name")),
      m_propertyShapeType(this, tr("Shape")),
      m_propertyXdeShapeKind(this, tr("XDE shape")),
      m_propertyColor(this, tr("Color")),
      m_propertyReferenceLocation(this, tr("Location")),
      m_propertyValidationCentroid(this, tr("Centroid")),
      m_propertyValidationArea(this, tr("Area")),
      m_propertyValidationVolume(this, tr("Volume")),
      m_propertyReferredName(this, tr("[Referred]Name")),
      m_propertyReferredColor(this, tr("[Referred]Color")),
      m_propertyReferredValidationCentroid(this, tr("[Referred]Centroid")),
      m_propertyReferredValidationArea(this, tr("[Referred]Area")),
      m_propertyReferredValidationVolume(this, tr("[Referred]Volume")),
      m_docItem(docItem),
      m_label(label)
{
    // Name
    m_propertyName.setValue(XdeDocumentItem::findLabelName(label));

    // Shape type
    const TopAbs_ShapeEnum shapeType = XCAFDoc_ShapeTool::GetShape(label).ShapeType();
    m_propertyShapeType.setValue(
                QString(StringUtils::rawText(shapeType)).remove("TopAbs_"));

    // XDE shape kind
    QStringList listXdeShapeKind;
    if (XdeDocumentItem::isShapeAssembly(label))
        listXdeShapeKind.push_back(tr("Assembly"));
    if (XdeDocumentItem::isShapeReference(label))
        listXdeShapeKind.push_back(tr("Reference"));
    if (XdeDocumentItem::isShapeComponent(label))
        listXdeShapeKind.push_back(tr("Component"));
    if (XdeDocumentItem::isShapeCompound(label))
        listXdeShapeKind.push_back(tr("Compound"));
    if (XdeDocumentItem::isShapeSimple(label))
        listXdeShapeKind.push_back(tr("Simple"));
    if (XdeDocumentItem::isShapeSub(label))
        listXdeShapeKind.push_back(tr("Sub"));
    m_propertyXdeShapeKind.setValue(listXdeShapeKind.join('+'));

    // Reference location
    if (XdeDocumentItem::isShapeReference(label)) {
        const TopLoc_Location loc = XdeDocumentItem::shapeReferenceLocation(label);
        m_propertyReferenceLocation.setValue(loc.Transformation());
    }
    else {
        this->removeProperty(&m_propertyReferenceLocation);
    }

    // Color
    if (docItem->hasShapeColor(label))
        m_propertyColor.setValue(docItem->shapeColor(label));
    else
        this->removeProperty(&m_propertyColor);

    // Validation properties
    {
        const XdeDocumentItem::ValidationProperties validProps =
                XdeDocumentItem::validationProperties(label);
        m_propertyValidationCentroid.setValue(validProps.centroid);
        if (!validProps.hasCentroid)
            this->removeProperty(&m_propertyValidationCentroid);
        m_propertyValidationArea.setQuantity(validProps.area);
        if (!validProps.hasArea)
            this->removeProperty(&m_propertyValidationArea);
        m_propertyValidationVolume.setQuantity(validProps.volume);
        if (!validProps.hasVolume)
            this->removeProperty(&m_propertyValidationVolume);
    }

    // Referred entity's properties
    if (XdeDocumentItem::isShapeReference(label)
            && opt == XdeDocumentItem::ShapePropertiesOption::MergeReferred)
    {
        m_labelReferred = XdeDocumentItem::shapeReferred(label);
        m_propertyReferredName.setValue(XdeDocumentItem::findLabelName(m_labelReferred));

        const XdeDocumentItem::ValidationProperties validProps =
                XdeDocumentItem::validationProperties(m_labelReferred);
        m_propertyReferredValidationCentroid.setValue(validProps.centroid);
        if (!validProps.hasCentroid)
            this->removeProperty(&m_propertyReferredValidationCentroid);
        m_propertyReferredValidationArea.setQuantity(validProps.area);
        if (!validProps.hasArea)
            this->removeProperty(&m_propertyReferredValidationArea);
        m_propertyReferredValidationVolume.setQuantity(validProps.volume);
        if (!validProps.hasVolume)
            this->removeProperty(&m_propertyReferredValidationVolume);

        if (docItem->hasShapeColor(m_labelReferred))
            m_propertyReferredColor.setValue(docItem->shapeColor(m_labelReferred));
        else
            this->removeProperty(&m_propertyReferredColor);
    }
    else {
        this->removeProperty(&m_propertyReferredName);
        this->removeProperty(&m_propertyReferredValidationCentroid);
        this->removeProperty(&m_propertyReferredValidationArea);
        this->removeProperty(&m_propertyReferredValidationVolume);
        this->removeProperty(&m_propertyReferredColor);
    }

    for (Property* prop : this->properties())
        prop->setUserReadOnly(true);
    m_propertyName.setUserReadOnly(false);
    m_propertyReferredName.setUserReadOnly(false);
}

const XdeDocumentItem *XdeShapePropertyOwner::xdeDocumentItem() const
{
    return m_docItem;
}

const TDF_Label &XdeShapePropertyOwner::label() const
{
    return m_label;
}

const TDF_Label &XdeShapePropertyOwner::referredLabel() const
{
    return m_labelReferred;
}

void XdeShapePropertyOwner::onPropertyChanged(Property *prop)
{
    if (prop == &m_propertyName) {
        XdeDocumentItem::setLabelName(m_label, m_propertyName.value());
        emit nameChanged();
    }
    else if (prop == &m_propertyReferredName) {
        XdeDocumentItem::setLabelName(m_labelReferred, m_propertyReferredName.value());
        emit referredNameChanged();
    }
    else {
        PropertyOwner::onPropertyChanged(prop);
    }
}

} // namespace Mayo
