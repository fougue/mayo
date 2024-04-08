/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_geom_curve.h"
#include "script_geom.h"

namespace Mayo {

ScriptGeomCurve::ScriptGeomCurve(const TopoDS_Edge& edge)
    : m_curve(edge)
{
}

int ScriptGeomCurve::intervalCount(unsigned continuity) const
{
    return m_curve.NbIntervals(static_cast<GeomAbs_Shape>(continuity));
}

QVariant ScriptGeomCurve::point(double u) const
{
    return ScriptGeom::toScriptValue(m_curve.Value(u));
}

QVariant ScriptGeomCurve::dN(double u, int n) const
{
    return ScriptGeom::toScriptValue(m_curve.DN(u, n));
}

} // namespace Mayo
