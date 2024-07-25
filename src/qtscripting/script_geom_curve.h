/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/occ_handle.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <BRepAdaptor_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Lin.hxx>
#include <gp_Parab.hxx>

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
    Q_PROPERTY(QVariant line READ line)
    Q_PROPERTY(QVariant circle READ circle)
    Q_PROPERTY(QVariant ellipse READ ellipse)
    Q_PROPERTY(QVariant hyperbola READ hyperbola)
    Q_PROPERTY(QVariant parabola READ parabola)
    Q_PROPERTY(QVariant bezier READ bezier)
    Q_PROPERTY(QVariant bspline READ bspline)
    Q_PROPERTY(QVariant offsetCurve READ offsetCurve)
public:
    ScriptGeomCurve() = default;
    ScriptGeomCurve(const OccHandle<Geom_Curve>& curve);
    ScriptGeomCurve(const TopoDS_Edge& edge);

    unsigned type() const { return this->curve().GetType(); }

    double paramFirst() const { return this->curve().FirstParameter(); }
    double paramLast() const { return this->curve().LastParameter(); }

    unsigned continuity() const { return this->curve().Continuity(); }

    Q_INVOKABLE int intervalCount(unsigned continuity) const;

    bool isClosed() const { return this->curve().IsClosed(); }

    bool isPeriodic() const { return this->curve().IsPeriodic(); }
    double period() const { return this->curve().Period(); }

    Q_INVOKABLE QVariant point(double u) const;     // ->{x, y, z}
    Q_INVOKABLE QVariant dN(double u, int n) const; // ->{x, y, z}

    QVariant line() const;      // ->ScriptGeomLine
    QVariant circle() const;    // ->ScriptGeomCircle
    QVariant ellipse() const;   // ->ScriptGeomEllipse
    QVariant hyperbola() const; // ->ScriptGeomHyperbola
    QVariant parabola() const;  // ->ScriptGeomParabola
    QVariant bezier() const;    // ->ScriptGeomBezierCurve
    QVariant bspline() const;   // ->ScriptGeomBSplineCurve
    QVariant offsetCurve() const; // ->ScriptGeomOffsetCurve

private:
    const Adaptor3d_Curve& curve() const;
    GeomAdaptor_Curve m_geomCurve;
    BRepAdaptor_Curve m_brepCurve;
};

class ScriptGeomLine {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
public:
    ScriptGeomLine() = default;
    ScriptGeomLine(const gp_Lin& lin);

    QVariant position() const; // ->ScriptGeomAx1

private:
    gp_Lin m_lin;
};

class ScriptGeomCircle {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double radius READ radius)
public:
    ScriptGeomCircle() = default;
    ScriptGeomCircle(const gp_Circ& circ);

    QVariant position() const; // ->ScriptGeomAx3
    double radius() const { return m_circ.Radius(); }

private:
    gp_Circ m_circ;
};

class ScriptGeomEllipse {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double majorRadius READ majorRadius)
    Q_PROPERTY(double minorRadius READ minorRadius)
    Q_PROPERTY(QVariant focus1 READ focus1)
    Q_PROPERTY(QVariant focus2 READ focus2)
    Q_PROPERTY(double focalDistance READ focalDistance)
    Q_PROPERTY(double parameter READ parameter)
public:
    ScriptGeomEllipse() = default;
    ScriptGeomEllipse(const gp_Elips& elips);

    QVariant position() const; // ->ScriptGeomAx3
    double majorRadius() const { return m_elips.MajorRadius(); }
    double minorRadius() const { return m_elips.MinorRadius(); }

    QVariant focus1() const; // ->{x, y, z}
    QVariant focus2() const; // ->{x, y, z}
    double focalDistance() const { return m_elips.Focal(); }
    double parameter() const { return m_elips.Parameter(); }

private:
    gp_Elips m_elips;
};

class ScriptGeomHyperbola {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double majorRadius READ majorRadius)
    Q_PROPERTY(double minorRadius READ minorRadius)
    Q_PROPERTY(QVariant focus1 READ focus1)
    Q_PROPERTY(QVariant focus2 READ focus2)
    Q_PROPERTY(double focalDistance READ focalDistance)
    Q_PROPERTY(double parameter READ parameter)
public:
    ScriptGeomHyperbola() = default;
    ScriptGeomHyperbola(const gp_Hypr& hypr);

    QVariant position() const; // ->ScriptGeomAx3
    double majorRadius() const { return m_hypr.MajorRadius(); }
    double minorRadius() const { return m_hypr.MinorRadius(); }

    QVariant focus1() const; // ->{x, y, z}
    QVariant focus2() const; // ->{x, y, z}
    double focalDistance() const { return m_hypr.Focal(); }
    double parameter() const { return m_hypr.Parameter(); }

private:
    gp_Hypr m_hypr;
};

class ScriptGeomParabola {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(QVariant focus READ focus)
    Q_PROPERTY(double focalDistance READ focalDistance)
    Q_PROPERTY(double parameter READ parameter)
public:
    ScriptGeomParabola() = default;
    ScriptGeomParabola(const gp_Parab& parab);

    QVariant position() const; // ->ScriptGeomAx3
    QVariant focus() const;    // ->{x, y, z}
    double focalDistance() const { return m_parab.Focal(); }
    double parameter() const { return m_parab.Parameter(); }

private:
    gp_Parab m_parab;
};

class ScriptGeomGeneralSplineCurve {
    Q_GADGET
    Q_PROPERTY(int degree READ degree)
    Q_PROPERTY(int poleCount READ poleCount)
    Q_PROPERTY(bool isRational READ isRational)
public:
    ScriptGeomGeneralSplineCurve() = default;

    int degree() const;
    int poleCount() const;
    bool isRational() const;

    Q_INVOKABLE QVariant pole(int index) const; // ->{x, y, z}
    Q_INVOKABLE double weight(int index) const;

protected:
    ScriptGeomGeneralSplineCurve(const OccHandle<Geom_BezierCurve>& bezier);
    ScriptGeomGeneralSplineCurve(const OccHandle<Geom_BSplineCurve>& bspline);

    const OccHandle<Geom_BezierCurve>& bezier() const { return m_bezier; }
    const OccHandle<Geom_BSplineCurve>& bspline() const { return m_bspline; }

    void throwIfNullSupportCurve() const;

private:
    OccHandle<Geom_BezierCurve> m_bezier;
    OccHandle<Geom_BSplineCurve> m_bspline;
};

class ScriptGeomBezierCurve : public ScriptGeomGeneralSplineCurve {
    Q_GADGET
public:
    ScriptGeomBezierCurve() = default;
    ScriptGeomBezierCurve(const OccHandle<Geom_BezierCurve>& bezier);
};

class ScriptGeomBSplineCurve : public ScriptGeomGeneralSplineCurve {
    Q_GADGET
    Q_PROPERTY(int knotIndexFirst READ knotIndexFirst)
    Q_PROPERTY(int knotIndexLast READ knotIndexLast)
    Q_PROPERTY(int knotCount READ knotCount)
    Q_PROPERTY(unsigned knotDistribution READ knotDistribution)
public:
    ScriptGeomBSplineCurve() = default;
    ScriptGeomBSplineCurve(const OccHandle<Geom_BSplineCurve>& bspline);

    Q_INVOKABLE double knot(int index) const;
    Q_INVOKABLE int multiplicity(int index) const;

    int knotIndexFirst() const;
    int knotIndexLast() const;
    int knotCount() const;

    unsigned knotDistribution() const; // ->GeomBSplineKnotDistribution
};

class ScriptGeomOffsetCurve {
    Q_GADGET
    Q_PROPERTY(QVariant basisCurve READ basisCurve)
    Q_PROPERTY(QVariant direction READ direction)
    Q_PROPERTY(double value READ value)
public:
    ScriptGeomOffsetCurve() = default;
    ScriptGeomOffsetCurve(const OccHandle<Geom_OffsetCurve>& offset);

    QVariant basisCurve() const; // ->ScriptGeomCurve
    QVariant direction() const;  // ->{x, y, z}
    double value() const;

private:
    OccHandle<Geom_OffsetCurve> m_offset;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomCurve)

Q_DECLARE_METATYPE(Mayo::ScriptGeomCircle)
Q_DECLARE_METATYPE(Mayo::ScriptGeomEllipse)
Q_DECLARE_METATYPE(Mayo::ScriptGeomHyperbola)
Q_DECLARE_METATYPE(Mayo::ScriptGeomParabola)
Q_DECLARE_METATYPE(Mayo::ScriptGeomBezierCurve)
Q_DECLARE_METATYPE(Mayo::ScriptGeomBSplineCurve)
Q_DECLARE_METATYPE(Mayo::ScriptGeomOffsetCurve)
