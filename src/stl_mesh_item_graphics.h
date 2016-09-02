#pragma once

#include "document_item_graphics.h"
#include "stl_mesh_item.h"
#include <MeshVS_Mesh.hxx>

namespace Mayo {

class StlMeshItemGraphics :
        public CovariantDocumentItemGraphics<StlMeshItem, MeshVS_Mesh, Handle_MeshVS_Mesh>
{
    Q_DECLARE_TR_FUNCTIONS(Mayo::StlMeshItemGraphics)

public:
    StlMeshItemGraphics(StlMeshItem* item);

    PropertyEnumeration propertyDisplayMode;
    PropertyBool propertyShowEdges;
    PropertyBool propertyShowNodes;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    static const Enumeration& enum_DisplayMode();
};

} // namespace Mayo
