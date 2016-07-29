#pragma once

#include <QtCore/QString>
#include <TopoDS_Shape.hxx>
#include <StlMesh_Mesh.hxx>

namespace Mayo {

class Part
{
public:
    Part() = default;
    Part(const TopoDS_Shape& shape);
    Part(const Handle_StlMesh_Mesh& mesh);

    const QString& filePath() const;
    void setFilePath(const QString& filepath);

    const TopoDS_Shape& brepShape() const;
    const Handle_StlMesh_Mesh& stlMesh() const;

    bool ownsBRepShape() const;
    bool ownsStlMesh() const;
    bool isNull() const;

private:
    TopoDS_Shape m_brepShape;
    Handle_StlMesh_Mesh m_stlMesh;
    QString m_filePath;
};

} // namespace Mayo
