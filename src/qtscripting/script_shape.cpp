/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_shape.h"

#include "script_geom_curve.h"
#include "script_geom_surface.h"

#include <QtCore/QVariant>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>

namespace Mayo {

bool ScriptShape::hasTransformation() const
{
    return !m_shape.Location().IsIdentity();
}

QVariantList ScriptShape::transformation() const
{
    const gp_Trsf& trsf = m_shape.Location().Transformation();
    QVariantList mat;
    for (int row = 1; row <= 3; ++row) {
        for (int col = 1; col <= 4; ++col)
            mat.push_back(trsf.Value(row, col));
    }

    mat.push_back(0.);
    mat.push_back(0.);
    mat.push_back(0.);
    mat.push_back(1.);
    return mat;
}

QVariant ScriptShape::geometry() const
{
    if (m_shape.ShapeType() == TopAbs_FACE) {
        const TopoDS_Face& face = TopoDS::Face(m_shape);
        return QVariant::fromValue(ScriptGeomSurface(face));
    }
    else if (m_shape.ShapeType() == TopAbs_EDGE) {
        const TopoDS_Edge& edge = TopoDS::Edge(m_shape);
        return QVariant::fromValue(ScriptGeomCurve(edge));
    }
    else if (m_shape.ShapeType() == TopAbs_VERTEX) {
        const TopoDS_Vertex& vertex = TopoDS::Vertex(m_shape);
        const gp_Pnt pnt = BRep_Tool::Pnt(vertex);
        QVariantMap map;
        map.insert("x", pnt.X());
        map.insert("y", pnt.Y());
        map.insert("z", pnt.Z());
        return map;
    }

    return {};
}

} // namespace Mayo
