/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "xde_document_item.h"
#include "gpx_document_item.h"
#include <XCAFPrs_AISObject.hxx>

namespace Mayo {

class GpxXdeDocumentItem : public GpxDocumentItem {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxXdeDocumentItem)
public:
    GpxXdeDocumentItem(XdeDocumentItem* item);
    ~GpxXdeDocumentItem();

    XdeDocumentItem* documentItem() const override;

    void display() override;
    void hide() override;
    void activateSelectionMode(int mode) override;
    std::vector<Handle_SelectMgr_EntityOwner> entityOwners(int mode) const override;
    Bnd_Box boundingBox() const override;

    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowFaceBoundary;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
    XdeDocumentItem* m_xdeDocItem = nullptr;
    std::vector<Handle_XCAFPrs_AISObject> m_vecXdeGpx;
};

} // namespace Mayo
