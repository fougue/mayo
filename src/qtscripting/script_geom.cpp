/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_geom.h"

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace Mayo {

ScriptGeomAx1::ScriptGeomAx1(const gp_Ax1& ax1)
    : m_ax1(ax1)
{
}

QVariant_Coords3D ScriptGeomAx1::location() const
{
    return ScriptGeom::toScriptValue(m_ax1.Location());
}

QVariant_Coords3D ScriptGeomAx1::direction() const
{
    return ScriptGeom::toScriptValue(m_ax1.Direction());
}


ScriptGeomAx3::ScriptGeomAx3(const gp_Ax3& ax3)
    : m_ax3(ax3)
{
}

QVariant_ScriptGeomAx1 ScriptGeomAx3::axis() const
{
    return ScriptGeom::toScriptValue(m_ax3.Axis());
}

QVariant_Coords3D ScriptGeomAx3::location() const
{
    return ScriptGeom::toScriptValue(m_ax3.Location());
}

QVariant_Coords3D ScriptGeomAx3::mainDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.Direction());
}

QVariant_Coords3D ScriptGeomAx3::xDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.XDirection());
}

QVariant_Coords3D ScriptGeomAx3::yDirection() const
{
    return ScriptGeom::toScriptValue(m_ax3.YDirection());
}


namespace ScriptGeom {

static QVariant_Coords3D xyz_toScriptValue(const gp_XYZ& coords)
{
    QVariantMap value;
    value.insert("x", coords.X());
    value.insert("y", coords.Y());
    value.insert("z", coords.Z());
    return value;
}

QVariant_Coords3D toScriptValue(const gp_Pnt& pnt)
{
    return xyz_toScriptValue(pnt.XYZ());
}

QVariant_Coords3D toScriptValue(const gp_Vec& vec)
{
    return xyz_toScriptValue(vec.XYZ());
}

QVariant_Coords3D toScriptValue(const gp_Dir& dir)
{
    return xyz_toScriptValue(dir.XYZ());
}

QVariant_ScriptGeomAx1 toScriptValue(const gp_Ax1& ax1)
{
    return QVariant::fromValue(ScriptGeomAx1(ax1));
}

QVariant_ScriptGeomAx3 toScriptValue(const gp_Ax3& ax3)
{
    return QVariant::fromValue(ScriptGeomAx3(ax3));
}

} // namespace ScriptGeom

} // namespace Mayo
