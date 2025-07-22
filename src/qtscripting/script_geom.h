/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>

namespace Mayo {

#ifndef _MAYO_DOCGEN_
using QVariant_Coords3D = QVariant;
using QVariant_ScriptGeomAx1 = QVariant;
using QVariant_ScriptGeomAx3 = QVariant;

using ScriptGeomContinuity = unsigned; // ->GeomAbs_Shape
using ScriptGeomBSplineKnotDistribution = unsigned; // ->GeomAbs_BSplKnotDistribution
#endif

class ScriptGeomAx1 {
    Q_GADGET
    Q_PROPERTY(QVariant_Coords3D location READ location)
    Q_PROPERTY(QVariant_Coords3D direction READ direction)
public:
    ScriptGeomAx1() = default;
    ScriptGeomAx1(const gp_Ax1& ax1);

    QVariant_Coords3D location() const;
    QVariant_Coords3D direction() const;

private:
    gp_Ax1 m_ax1;
};

class ScriptGeomAx3 {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx1 axis READ axis)
    Q_PROPERTY(QVariant_Coords3D location READ location)
    Q_PROPERTY(QVariant_Coords3D mainDirection READ mainDirection)
    Q_PROPERTY(QVariant_Coords3D xDirection READ xDirection)
    Q_PROPERTY(QVariant_Coords3D yDirection READ yDirection)
public:
    ScriptGeomAx3() = default;
    ScriptGeomAx3(const gp_Ax3& ax3);

    QVariant_ScriptGeomAx1 axis() const;
    QVariant_Coords3D location() const;
    QVariant_Coords3D mainDirection() const;
    QVariant_Coords3D xDirection() const;
    QVariant_Coords3D yDirection() const;

private:
    gp_Ax3 m_ax3;
};

namespace ScriptGeom {

QVariant_Coords3D toScriptValue(const gp_Pnt& pnt);
QVariant_Coords3D toScriptValue(const gp_Vec& vec);
QVariant_Coords3D toScriptValue(const gp_Dir& dir);
QVariant_ScriptGeomAx1 toScriptValue(const gp_Ax1& ax1);
QVariant_ScriptGeomAx3 toScriptValue(const gp_Ax3& ax3);

} // namespace ScriptGeom
} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomAx1)
Q_DECLARE_METATYPE(Mayo::ScriptGeomAx3)
