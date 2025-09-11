/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_shape.h"

#include "../base/brep_utils.h"
#include "script_geom_curve.h"
#include "script_geom_surface.h"
#include "script_global.h"

#include <QtCore/QVariant>
#include <QtQml/QJSEngine>

#include <BRep_Tool.hxx>
#include <TopoDS.hxx>

namespace Mayo {

ScriptShape::ScriptShape(const TopoDS_Shape& shape, QJSEngine* jsEngine)
    : m_shape(shape),
    m_jsEngine(jsEngine)
{
}

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
        return ScriptGeom::toScriptValue(BRep_Tool::Pnt(vertex));
    }

    return {};
}

//! \brief Visits each sub-shape and executes callback `fn` on the visited sub-shape
//!
//! The sub-shapes visited are restricted by a shape type filter specified with `filter`.
//! This means that if for example ShapeType.Edge is passed then only the sub-shapes of type "edge"
//! will be visited\n
//! `ShapeTraverseCallback` is a unary callback function which is passed the Shape object of the
//! visited sub-shape. Any value returned by the callback is ignored
//! \code{.js}
//! var circleCount = 0;
//! shape.traverse(ShapeType.Edge, edge => {
//!     if (edge.geometry.type == GeomCurveType.Circle)
//!         ++circleCount;
//! });
//! console.debug("Circle count = " + circleCount);
//! \endcode
void ScriptShape::traverse(ScriptShapeType filter, QJSValue_ShapeTraverseCallback fn)
{
    if (!m_jsEngine)
        return; // TODO Handle error(throw exception?)

    if (!fn.isCallable())
        return;

    const auto shapeTypeEnum = static_cast<TopAbs_ShapeEnum>(filter);
    BRepUtils::forEachSubShape(m_shape, shapeTypeEnum, [&](const TopoDS_Shape& subShape) {
        auto jsSubShape = m_jsEngine->toScriptValue(ScriptShape(subShape));
        auto jsVal = fn.call({ jsSubShape });
        logScriptError(jsVal, "Shape.traverse()");
    });
}

} // namespace Mayo
