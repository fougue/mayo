#pragma once

#include "document_item.h"
#include <StlMesh_Mesh.hxx>

namespace Mayo {

class StlMeshItem : public PartItem
{
public:
    const Handle_StlMesh_Mesh& stlMesh() const;
    void setStlMesh(const Handle_StlMesh_Mesh& mesh);

    bool isNull() const override;

    static const char* type;
    const char* dynType() const override;

private:
    Handle_StlMesh_Mesh m_stlMesh;
};

} // namespace Mayo
