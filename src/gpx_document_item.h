/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_builtins.h"
#include "property_enumeration.h"

#include <QtCore/QCoreApplication>
#include <AIS_InteractiveObject.hxx>

namespace Mayo {

class DocumentItem;

class GpxDocumentItem : public PropertyOwner {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxDocumentItem)
public:
    GpxDocumentItem();
    virtual ~GpxDocumentItem() = default;

    virtual DocumentItem* documentItem() const = 0;
    virtual Handle_AIS_InteractiveObject handleGpxObject() const = 0;

    PropertyBool propertyIsVisible;
    PropertyEnumeration propertyMaterial;
    PropertyOccColor propertyColor;

protected:
    void onPropertyChanged(Property* prop) override;
};

} // namespace Mayo
