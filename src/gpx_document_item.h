/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_builtins.h"
#include "property_enumeration.h"

#include <QtCore/QCoreApplication>
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>

namespace Mayo {

class DocumentItem;

class GpxDocumentItem : public PropertyOwner {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxDocumentItem)
public:
    GpxDocumentItem();
    virtual ~GpxDocumentItem() = default;

    virtual DocumentItem* documentItem() const = 0;

    const Handle_AIS_InteractiveContext& context() const { return m_ctx; }
    void setContext(const Handle_AIS_InteractiveContext& ctx) { m_ctx = ctx; }

    virtual void setVisible(bool on) = 0;
    virtual void activateSelection(int mode);
    virtual std::vector<Handle_SelectMgr_EntityOwner> entityOwners(int mode) const;
    virtual Bnd_Box boundingBox() const = 0;

    PropertyBool propertyIsVisible;
    PropertyEnumeration propertyMaterial;
    PropertyOccColor propertyColor;

protected:
    void onPropertyChanged(Property* prop) override;
    static void getEntityOwners(
            const Handle_AIS_InteractiveContext& ctx,
            const Handle_AIS_InteractiveObject& obj,
            int mode,
            std::vector<Handle_SelectMgr_EntityOwner>* vec);

private:
    Handle_AIS_InteractiveContext m_ctx;
};

} // namespace Mayo
