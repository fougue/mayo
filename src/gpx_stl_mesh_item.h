#pragma once

#include "gpx_document_item.h"
#include "stl_mesh_item.h"
#include <MeshVS_Mesh.hxx>

namespace Mayo {

class GpxStlMeshItem :
        public GpxCovariantDocumentItem<StlMeshItem, MeshVS_Mesh, Handle_MeshVS_Mesh>
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxStlMeshItem)

public:
    GpxStlMeshItem(StlMeshItem* item);

    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowEdges;
    PropertyBool propertyShowNodes;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
};

} // namespace Mayo
