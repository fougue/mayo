#include "stl_mesh_item.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

StlMeshItem::StlMeshItem()
    : propertyNodeCount(
          this, QCoreApplication::translate("Mayo::StlMeshItem", "Node count")),
      propertyTriangleCount(
          this, QCoreApplication::translate("Mayo::StlMeshItem", "Triangle count"))
{
}

const Handle_StlMesh_Mesh &StlMeshItem::stlMesh() const
{
    return m_stlMesh;
}

void StlMeshItem::setStlMesh(const Handle_StlMesh_Mesh &mesh)
{
    m_stlMesh = mesh;
}

bool StlMeshItem::isNull() const
{
    return m_stlMesh.IsNull();
}

const char* StlMeshItem::type = "2d441323-48db-4222-91b4-bdb7b5460c3f";
const char* StlMeshItem::dynType() const { return StlMeshItem::type; }

} // namespace Mayo
