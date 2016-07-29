#include "part.h"

namespace Mayo {

Part::Part(const TopoDS_Shape &shape)
    : m_brepShape(shape)
{ }

Part::Part(const Handle_StlMesh_Mesh &mesh)
    : m_stlMesh(mesh)
{ }

const QString &Part::filePath() const
{
    return m_filePath;
}

void Part::setFilePath(const QString &filepath)
{
    m_filePath = filepath;
}

const TopoDS_Shape &Part::brepShape() const
{
    return m_brepShape;
}

const Handle_StlMesh_Mesh &Part::stlMesh() const
{
    return m_stlMesh;
}

bool Part::ownsBRepShape() const
{
    return !m_brepShape.IsNull();
}

bool Part::ownsStlMesh() const
{
    return !m_stlMesh.IsNull();
}

bool Part::isNull() const
{
    return !this->ownsBRepShape() && !this->ownsStlMesh();
}

} // namespace Mayo
