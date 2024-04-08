/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_geom.h"

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace Mayo {

ScriptGeomAx3::ScriptGeomAx3(const gp_Ax3& ax3)
    : m_ax3(ax3)
{
}

QVariant ScriptGeomAx3::location() const
{
    return ScriptGeom::toScriptValue(m_ax3.Location());
}

QVariant ScriptGeomAx3::mainDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.Direction());
}

QVariant ScriptGeomAx3::xDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.XDirection());
}

QVariant ScriptGeomAx3::yDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.YDirection());
}

namespace ScriptGeom {

static QVariant xyz_toScriptValue(const gp_XYZ& coords)
{
    QVariantMap value;
    value.insert("x", coords.X());
    value.insert("y", coords.Y());
    value.insert("z", coords.Z());
    return value;
}

QVariant toScriptValue(const gp_Pnt& pnt)
{
    return xyz_toScriptValue(pnt.XYZ());
}

QVariant toScriptValue(const gp_Vec& vec)
{
    return xyz_toScriptValue(vec.XYZ());
}

QVariant toScriptValue(const gp_Dir& dir)
{
    return xyz_toScriptValue(dir.XYZ());
}

QVariant toScriptValue(const gp_Ax3& ax3)
{
    return QVariant::fromValue(ScriptGeomAx3(ax3));
}

} // namespace ScriptGeom

} // namespace Mayo
