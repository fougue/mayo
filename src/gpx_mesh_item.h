/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "gpx_document_item.h"
#include "mesh_item.h"
#include <MeshVS_Mesh.hxx>

namespace Mayo {

class GpxMeshItem : public GpxDocumentItem {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxMeshItem)
public:
    GpxMeshItem(MeshItem* item);

    MeshItem* documentItem() const override;
    Handle_AIS_InteractiveObject handleGpxObject() const override;

    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowEdges;
    PropertyBool propertyShowNodes;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
    MeshItem* m_meshItem = nullptr;
    Handle_MeshVS_Mesh m_meshVisu;
};

} // namespace Mayo
