/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "property_builtins.h"
#include "xde_document_item.h"

namespace Mayo {

class XdeShapePropertyOwner : public PropertyOwnerSignals {
    Q_OBJECT
public:
    const XdeDocumentItem* xdeDocumentItem() const;
    const TDF_Label& label() const;
    const TDF_Label& referredLabel() const;

signals:
    void nameChanged();
    void referredNameChanged();

protected:
    void onPropertyChanged(Property* prop) override;

private:
    XdeShapePropertyOwner(const XdeDocumentItem* docItem, const TDF_Label& label);

    friend class XdeDocumentItem;

    PropertyQString m_propertyName;
    PropertyQString m_propertyShapeType;
    PropertyQString m_propertyXdeShapeKind;
    PropertyOccColor m_propertyColor;
    PropertyOccTrsf m_propertyReferenceLocation;
    PropertyOccPnt m_propertyValidationCentroid;
    PropertyArea m_propertyValidationArea;
    PropertyVolume m_propertyValidationVolume;

    PropertyQString m_propertyReferredName;
    PropertyOccColor m_propertyReferredColor;
    PropertyOccPnt m_propertyReferredValidationCentroid;
    PropertyArea m_propertyReferredValidationArea;
    PropertyVolume m_propertyReferredValidationVolume;

    const XdeDocumentItem* m_docItem = nullptr;
    TDF_Label m_label;
    TDF_Label m_labelReferred;
};

} // namespace Mayo
