/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mesh_item.h"

#include <QtCore/QCoreApplication>

namespace Mayo {

MeshItem::MeshItem()
    : propertyNodeCount(
          this, QCoreApplication::translate("Mayo::MeshItem", "Node count")),
      propertyTriangleCount(
          this, QCoreApplication::translate("Mayo::MeshItem", "Triangle count"))
{
    this->propertyNodeCount.setUserReadOnly(true);
    this->propertyTriangleCount.setUserReadOnly(true);
}

const Handle_Poly_Triangulation& MeshItem::triangulation() const
{
    return m_triangulation;
}

void MeshItem::setTriangulation(const Handle_Poly_Triangulation& mesh)
{
    m_triangulation = mesh;
}

bool MeshItem::isNull() const
{
    return m_triangulation.IsNull();
}

const char MeshItem::TypeName[] = "2d441323-48db-4222-91b4-bdb7b5460c3f";
const char* MeshItem::dynTypeName() const { return MeshItem::TypeName; }

} // namespace Mayo
