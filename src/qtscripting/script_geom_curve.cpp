/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_geom_curve.h"
#include "script_geom.h"
#include "../base/cpp_utils.h"

#include <stdexcept>

namespace Mayo {

ScriptGeomCurve::ScriptGeomCurve(const OccHandle<Geom_Curve>& curve)
    : m_geomCurve(curve)
{
}

ScriptGeomCurve::ScriptGeomCurve(const TopoDS_Edge& edge)
    : m_brepCurve(edge)
{
}

int ScriptGeomCurve::intervalCount(unsigned continuity) const
{
    return this->curve().NbIntervals(static_cast<GeomAbs_Shape>(continuity));
}

QVariant ScriptGeomCurve::point(double u) const
{
    return ScriptGeom::toScriptValue(this->curve().Value(u));
}

QVariant ScriptGeomCurve::dN(double u, int n) const
{
    return ScriptGeom::toScriptValue(this->curve().DN(u, n));
}

QVariant ScriptGeomCurve::line() const
{
    return QVariant::fromValue(ScriptGeomLine(this->curve().Line()));
}

QVariant ScriptGeomCurve::circle() const
{
    return QVariant::fromValue(ScriptGeomCircle(this->curve().Circle()));
}

QVariant ScriptGeomCurve::ellipse() const
{
    return QVariant::fromValue(ScriptGeomEllipse(this->curve().Ellipse()));
}

QVariant ScriptGeomCurve::hyperbola() const
{
    return QVariant::fromValue(ScriptGeomHyperbola(this->curve().Hyperbola()));
}

QVariant ScriptGeomCurve::parabola() const
{
    return QVariant::fromValue(ScriptGeomParabola(this->curve().Parabola()));
}

QVariant ScriptGeomCurve::bezier() const
{
    return QVariant::fromValue(ScriptGeomBezierCurve(this->curve().Bezier()));
}

QVariant ScriptGeomCurve::bspline() const
{
    return QVariant::fromValue(ScriptGeomBSplineCurve(this->curve().BSpline()));
}

QVariant ScriptGeomCurve::offsetCurve() const
{
    return QVariant::fromValue(ScriptGeomOffsetCurve(this->curve().OffsetCurve()));
}

const Adaptor3d_Curve& ScriptGeomCurve::curve() const
{
    if (m_geomCurve.Curve())
        return m_geomCurve;

    if (!m_brepCurve.Edge().IsNull())
        return m_brepCurve;

    throw std::runtime_error("No support curve");
}


ScriptGeomLine::ScriptGeomLine(const gp_Lin& lin)
    : m_lin(lin)
{
}

QVariant ScriptGeomLine::position() const
{
    return ScriptGeom::toScriptValue(m_lin.Position());
}


ScriptGeomCircle::ScriptGeomCircle(const gp_Circ& circ)
    : m_circ(circ)
{
}

QVariant ScriptGeomCircle::position() const
{
    return ScriptGeom::toScriptValue(m_circ.Position());
}


ScriptGeomEllipse::ScriptGeomEllipse(const gp_Elips& elips)
    : m_elips(elips)
{
}

QVariant ScriptGeomEllipse::position() const
{
    return ScriptGeom::toScriptValue(m_elips.Position());
}

QVariant ScriptGeomEllipse::focus1() const
{
    return ScriptGeom::toScriptValue(m_elips.Focus1());
}

QVariant ScriptGeomEllipse::focus2() const
{
    return ScriptGeom::toScriptValue(m_elips.Focus2());
}


ScriptGeomHyperbola::ScriptGeomHyperbola(const gp_Hypr& hypr)
    : m_hypr(hypr)
{
}

QVariant ScriptGeomHyperbola::position() const
{
    return ScriptGeom::toScriptValue(m_hypr.Position());
}

QVariant ScriptGeomHyperbola::focus1() const
{
    return ScriptGeom::toScriptValue(m_hypr.Focus1());
}

QVariant ScriptGeomHyperbola::focus2() const
{
    return ScriptGeom::toScriptValue(m_hypr.Focus2());
}


ScriptGeomParabola::ScriptGeomParabola(const gp_Parab& parab)
    : m_parab(parab)
{
}

QVariant ScriptGeomParabola::position() const
{
    return ScriptGeom::toScriptValue(m_parab.Position());
}

QVariant ScriptGeomParabola::focus() const
{
    return ScriptGeom::toScriptValue(m_parab.Focus());
}


ScriptGeomGeneralSplineCurve::ScriptGeomGeneralSplineCurve(const OccHandle<Geom_BezierCurve>& bezier)
    : m_bezier(bezier)
{
    this->throwIfNullSupportCurve();
}

ScriptGeomGeneralSplineCurve::ScriptGeomGeneralSplineCurve(const OccHandle<Geom_BSplineCurve>& bspline)
    : m_bspline(bspline)
{
    this->throwIfNullSupportCurve();
}

int ScriptGeomGeneralSplineCurve::degree() const
{
    if (m_bezier)
        return m_bezier->Degree();
    else if (m_bspline)
        return m_bspline->Degree();

    return 0;
}

int ScriptGeomGeneralSplineCurve::poleCount() const
{
    if (m_bezier)
        return m_bezier->NbPoles();
    else if (m_bspline)
        return m_bspline->NbPoles();

    return 0;
}

bool ScriptGeomGeneralSplineCurve::isRational() const
{
    if (m_bezier)
        return m_bezier->IsRational();
    else if (m_bspline)
        return m_bspline->IsRational();

    return false;
}

QVariant ScriptGeomGeneralSplineCurve::pole(int index) const
{
    this->throwIfNullSupportCurve();
    if (m_bezier)
        return ScriptGeom::toScriptValue(m_bezier->Pole(index));
    else if (m_bspline)
        return ScriptGeom::toScriptValue(m_bspline->Pole(index));

    return {};
}

double ScriptGeomGeneralSplineCurve::weight(int index) const
{
    this->throwIfNullSupportCurve();
    if (m_bezier)
        return m_bezier->Weight(index);
    else if (m_bspline)
        return m_bspline->Weight(index);

    return 0.;
}

void ScriptGeomGeneralSplineCurve::throwIfNullSupportCurve() const
{
    if (!m_bezier && !m_bspline)
        throw std::runtime_error("Geom_BezierCurve and Geom_BSplineCurve pointers are null");
}


ScriptGeomBezierCurve::ScriptGeomBezierCurve(const OccHandle<Geom_BezierCurve>& bezier)
    : ScriptGeomGeneralSplineCurve(bezier)
{
}


ScriptGeomBSplineCurve::ScriptGeomBSplineCurve(const OccHandle<Geom_BSplineCurve>& bspline)
    : ScriptGeomGeneralSplineCurve(bspline)
{
}

double ScriptGeomBSplineCurve::knot(int index) const
{
    return this->bspline()->Knot(index);
}

int ScriptGeomBSplineCurve::multiplicity(int index) const
{
    return this->bspline()->Multiplicity(index);
}

int ScriptGeomBSplineCurve::knotIndexFirst() const
{
    return this->bspline()->FirstUKnotIndex();
}

int ScriptGeomBSplineCurve::knotIndexLast() const
{
    return this->bspline()->LastUKnotIndex();
}

int ScriptGeomBSplineCurve::knotCount() const
{
    return this->bspline()->NbKnots();
}

unsigned ScriptGeomBSplineCurve::knotDistribution() const
{
    return this->bspline()->KnotDistribution();
}


ScriptGeomOffsetCurve::ScriptGeomOffsetCurve(const OccHandle<Geom_OffsetCurve>& offset)
    : m_offset(offset)
{
}

QVariant ScriptGeomOffsetCurve::basisCurve() const
{
    Cpp::throwErrorIf<std::runtime_error>(!m_offset, "Geom_OffsetCurve pointer is null");
    return QVariant::fromValue(ScriptGeomCurve(m_offset));
}

QVariant ScriptGeomOffsetCurve::direction() const
{
    Cpp::throwErrorIf<std::runtime_error>(!m_offset, "Geom_OffsetCurve pointer is null");
    return ScriptGeom::toScriptValue(m_offset->Direction());
}

double ScriptGeomOffsetCurve::value() const
{
    Cpp::throwErrorIf<std::runtime_error>(!m_offset, "Geom_OffsetCurve pointer is null");
    return m_offset->Offset();
}

} // namespace Mayo
