/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <gp_Ax3.hxx>

namespace Mayo {

class ScriptGeomAx3 {
    Q_GADGET
    Q_PROPERTY(QVariant location READ location)
    Q_PROPERTY(QVariant mainDirection READ mainDirection)
    Q_PROPERTY(QVariant xDirection READ xDirection)
    Q_PROPERTY(QVariant yDirection READ yDirection)
public:
    ScriptGeomAx3() = default;
    ScriptGeomAx3(const gp_Ax3& ax3);

    QVariant location() const;
    QVariant mainDirection() const;
    QVariant xDirection() const;
    QVariant yDirection() const;

private:
    gp_Ax3 m_ax3;
};

namespace ScriptGeom {

QVariant toScriptValue(const gp_Pnt& pnt);
QVariant toScriptValue(const gp_Vec& vec);
QVariant toScriptValue(const gp_Dir& dir);
QVariant toScriptValue(const gp_Ax3& ax3);

} // namespace ScriptGeom
} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomAx3)
