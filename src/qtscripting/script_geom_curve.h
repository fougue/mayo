/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <BRepAdaptor_Curve.hxx>
#include <QtCore/QObject>
#include <QtCore/QVariant>

namespace Mayo {

class ScriptGeomCurve {
    Q_GADGET
    Q_PROPERTY(unsigned type READ type)
    Q_PROPERTY(double paramFirst READ paramFirst)
    Q_PROPERTY(double paramLast READ paramLast)
    Q_PROPERTY(unsigned continuity READ continuity)
    Q_PROPERTY(bool isClosed READ isClosed)
    Q_PROPERTY(bool isPeriodic READ isPeriodic)
    Q_PROPERTY(double period READ period)
public:
    ScriptGeomCurve() = default;
    ScriptGeomCurve(const TopoDS_Edge& edge);

    unsigned type() const { return m_curve.GetType(); }

    double paramFirst() const { return m_curve.FirstParameter(); }
    double paramLast() const { return m_curve.LastParameter(); }

    unsigned continuity() const { return m_curve.Continuity(); }

    Q_INVOKABLE int intervalCount(unsigned continuity) const;

    bool isClosed() const { return m_curve.IsClosed(); }

    bool isPeriodic() const { return m_curve.IsPeriodic(); }

    double period() const { return m_curve.Period(); }

    Q_INVOKABLE QVariant point(double u) const;
    Q_INVOKABLE QVariant dN(double u, int n) const;

private:
    BRepAdaptor_Curve m_curve;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomCurve)
