/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
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

    void setVisible(bool on) override;
    void activateSelection(int mode) override;
    std::vector<Handle_SelectMgr_EntityOwner> entityOwners(int mode) const override;
    Bnd_Box boundingBox() const override;

    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;

    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };
    static const Enumeration& enumDisplayMode();

protected:
    void onPropertyChanged(Property* prop) override;

private:
    XdeDocumentItem* m_xdeDocItem = nullptr;
    std::vector<Handle_XCAFPrs_AISObject> m_vecXdeGpx;
};

} // namespace Mayo
