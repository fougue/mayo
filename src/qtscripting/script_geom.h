/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>

namespace Mayo {

class ScriptGeomAx1 {
    Q_GADGET
    Q_PROPERTY(QVariant location READ location)
    Q_PROPERTY(QVariant direction READ direction)
public:
    ScriptGeomAx1() = default;
    ScriptGeomAx1(const gp_Ax1& ax1);

    QVariant location() const;  // ->{x, y, z}
    QVariant direction() const; // ->{x, y, z}

private:
    gp_Ax1 m_ax1;
};

class ScriptGeomAx3 {
    Q_GADGET
    Q_PROPERTY(QVariant location READ location)
    Q_PROPERTY(QVariant mainDirection READ mainDirection)
    Q_PROPERTY(QVariant xDirection READ xDirection)
    Q_PROPERTY(QVariant yDirection READ yDirection)
public:
    ScriptGeomAx3() = default;
    ScriptGeomAx3(const gp_Ax3& ax3);

    QVariant location() const;      // ->{x, y, z}
    QVariant mainDirection() const; // ->{x, y, z}
    QVariant xDirection() const;    // ->{x, y, z}
    QVariant yDirection() const;    // ->{x, y, z}

private:
    gp_Ax3 m_ax3;
};

namespace ScriptGeom {

QVariant toScriptValue(const gp_Pnt& pnt); // ->{x, y, z}
QVariant toScriptValue(const gp_Vec& vec); // ->{x, y, z}
QVariant toScriptValue(const gp_Dir& dir); // ->{x, y, z}
QVariant toScriptValue(const gp_Ax1& ax1); // ->ScriptGeomAx1
QVariant toScriptValue(const gp_Ax3& ax3); // ->ScriptGeomAx3

} // namespace ScriptGeom
} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomAx3)
