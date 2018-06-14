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
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class DocumentItem;

class GpxDocumentItem : public PropertyOwner {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxDocumentItem)
public:
    GpxDocumentItem();

    virtual DocumentItem* documentItem() const = 0;
    virtual Handle_AIS_InteractiveObject handleGpxObject() const = 0;
    virtual AIS_InteractiveObject* gpxObject() const = 0;

    PropertyBool propertyIsVisible;
    PropertyEnumeration propertyMaterial;
    PropertyOccColor propertyColor;

protected:
    void onPropertyChanged(Property* prop) override;

    void initForGpxBRepShape(const Handle_AIS_InteractiveObject& hndGpx);
};

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
class GpxCovariantDocumentItem : public GpxDocumentItem
{
public:
    GpxCovariantDocumentItem(DOC_ITEM* item);
    GpxCovariantDocumentItem(DOC_ITEM* item, const HND_GPX_OBJECT& hndGpx);

    DOC_ITEM* documentItem() const override;
    Handle_AIS_InteractiveObject handleGpxObject() const override;
    GPX_OBJECT* gpxObject() const override;

protected:
    DOC_ITEM* m_docItem = nullptr;
    HND_GPX_OBJECT m_hndGpxObject;
};

class GpxBRepShapeCommonProperties
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxBRepShapeCommonProperties)

public:
    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowFaceBoundary;

protected:
    GpxBRepShapeCommonProperties(PropertyOwner* owner);

    void initCommonProperties(
            PropertyOwner* owner, const Handle_AIS_InteractiveObject& hndGpx);
    void handleCommonPropertyChange(
            Property* prop, const Handle_AIS_InteractiveObject& hndGpx);
    void handlePropertyMaterial(
            PropertyEnumeration* prop, const Handle_AIS_InteractiveObject& hndGpx);
    void handlePropertyColor(
            PropertyOccColor* prop, const Handle_AIS_InteractiveObject& hndGpx);

private:
    static const Enumeration& enum_DisplayMode();
};

// --
// -- Implementation
// --

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
GpxCovariantDocumentItem<DOC_ITEM, GPX_OBJECT, HND_GPX_OBJECT>::
GpxCovariantDocumentItem(DOC_ITEM* item)
    : m_docItem(item)
{ }

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
GpxCovariantDocumentItem<DOC_ITEM, GPX_OBJECT, HND_GPX_OBJECT>::
GpxCovariantDocumentItem(DOC_ITEM* item, const HND_GPX_OBJECT& hndGpx)
    : m_docItem(item),
      m_hndGpxObject(hndGpx)
{ }

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
DOC_ITEM* GpxCovariantDocumentItem<DOC_ITEM, GPX_OBJECT, HND_GPX_OBJECT>::documentItem() const
{ return m_docItem; }

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
Handle_AIS_InteractiveObject
GpxCovariantDocumentItem<DOC_ITEM, GPX_OBJECT, HND_GPX_OBJECT>::handleGpxObject() const
{ return m_hndGpxObject; }

template<typename DOC_ITEM, typename GPX_OBJECT, typename HND_GPX_OBJECT>
GPX_OBJECT* GpxCovariantDocumentItem<DOC_ITEM, GPX_OBJECT, HND_GPX_OBJECT>::gpxObject() const
{ return m_hndGpxObject.operator->(); }

} // namespace Mayo
