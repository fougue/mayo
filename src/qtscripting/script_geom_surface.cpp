/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_geom_surface.h"
#include "script_geom.h"

#include <stdexcept>

namespace Mayo {

ScriptGeomSurface::ScriptGeomSurface(const TopoDS_Face& face)
    : m_surface(face)
{
}

int ScriptGeomSurface::uIntervalCount(unsigned continuity) const
{
    return m_surface.NbUIntervals(static_cast<GeomAbs_Shape>(continuity));
}

int ScriptGeomSurface::vIntervalCount(unsigned continuity) const
{
    return m_surface.NbVIntervals(static_cast<GeomAbs_Shape>(continuity));
}

QVariant ScriptGeomSurface::point(double u, double v) const
{
    return ScriptGeom::toScriptValue(m_surface.Value(u, v));
}

QVariant ScriptGeomSurface::dN(double u, double v, int uN, int vN) const
{
    return ScriptGeom::toScriptValue(m_surface.DN(u, v, uN, vN));
}

QVariant ScriptGeomSurface::cylinder() const
{
    return QVariant::fromValue(ScriptGeomCylinder(m_surface.Cylinder()));
}

QVariant ScriptGeomSurface::plane() const
{
    return QVariant::fromValue(ScriptGeomPlane(m_surface.Plane()));
}

QVariant ScriptGeomSurface::cone() const
{
    return QVariant::fromValue(ScriptGeomCone(m_surface.Cone()));
}

QVariant ScriptGeomSurface::sphere() const
{
    return QVariant::fromValue(ScriptGeomSphere(m_surface.Sphere()));
}

QVariant ScriptGeomSurface::torus() const
{
    return QVariant::fromValue(ScriptGeomTorus(m_surface.Torus()));
}

QVariant ScriptGeomSurface::bezier() const
{
    return QVariant::fromValue(ScriptGeomBezierSurface(m_surface.Bezier()));
}

QVariant ScriptGeomSurface::bspline() const
{
    return QVariant::fromValue(ScriptGeomBSplineSurface(m_surface.BSpline()));
}


ScriptGeomCylinder::ScriptGeomCylinder(const gp_Cylinder& cyl)
    : m_cyl(cyl)
{
}

QVariant ScriptGeomCylinder::position() const
{
    return ScriptGeom::toScriptValue(m_cyl.Position());
}


ScriptGeomPlane::ScriptGeomPlane(const gp_Pln& pln)
    : m_pln(pln)
{
}

QVariant ScriptGeomPlane::position() const
{
    return ScriptGeom::toScriptValue(m_pln.Position());
}


ScriptGeomCone::ScriptGeomCone(const gp_Cone& cone)
    : m_cone(cone)
{
}

QVariant ScriptGeomCone::position() const
{
    return ScriptGeom::toScriptValue(m_cone.Position());
}

QVariant ScriptGeomCone::apex() const
{
    return ScriptGeom::toScriptValue(m_cone.Apex());
}


ScriptGeomSphere::ScriptGeomSphere(const gp_Sphere& sphere)
    : m_sphere(sphere)
{
}

QVariant ScriptGeomSphere::position() const
{
    return ScriptGeom::toScriptValue(m_sphere.Position());
}


ScriptGeomTorus::ScriptGeomTorus(const gp_Torus& torus)
    : m_torus(torus)
{
}

QVariant ScriptGeomTorus::position() const
{
    return ScriptGeom::toScriptValue(m_torus.Position());
}


ScriptGeomGeneralSplineSurface::ScriptGeomGeneralSplineSurface(const OccHandle<Geom_BezierSurface>& bezier)
    : m_bezier(bezier)
{
    this->throwIfNullSupportSurface();
}

ScriptGeomGeneralSplineSurface::ScriptGeomGeneralSplineSurface(const OccHandle<Geom_BSplineSurface>& bspline)
    : m_bspline(bspline)
{
    this->throwIfNullSupportSurface();
}

int ScriptGeomGeneralSplineSurface::uDegree() const
{
    if (m_bezier)
        return m_bezier->UDegree();
    else if (m_bspline)
        return m_bspline->UDegree();

    return 0;
}

int ScriptGeomGeneralSplineSurface::vDegree() const
{
    if (m_bezier)
        return m_bezier->VDegree();
    else if (m_bspline)
        return m_bspline->VDegree();

    return 0;
}

int ScriptGeomGeneralSplineSurface::uPoleCount() const
{
    if (m_bezier)
        return m_bezier->NbUPoles();
    else if (m_bspline)
        return m_bspline->NbUPoles();

    return 0;
}

int ScriptGeomGeneralSplineSurface::vPoleCount() const
{
    if (m_bezier)
        return m_bezier->NbVPoles();
    else if (m_bspline)
        return m_bspline->NbVPoles();

    return 0;
}

bool ScriptGeomGeneralSplineSurface::isURational() const
{
    if (m_bezier)
        return m_bezier->IsURational();
    else if (m_bspline)
        return m_bspline->IsURational();

    return false;
}

bool ScriptGeomGeneralSplineSurface::isVRational() const
{
    if (m_bezier)
        return m_bezier->IsVRational();
    else if (m_bspline)
        return m_bspline->IsVRational();

    return false;
}

QVariant ScriptGeomGeneralSplineSurface::pole(int uIndex, int vIndex) const
{
    this->throwIfNullSupportSurface();
    if (m_bezier)
        return ScriptGeom::toScriptValue(m_bezier->Pole(uIndex, vIndex));
    else if (m_bspline)
        return ScriptGeom::toScriptValue(m_bspline->Pole(uIndex, vIndex));

    return {};
}

double ScriptGeomGeneralSplineSurface::weight(int uIndex, int vIndex) const
{
    this->throwIfNullSupportSurface();
    if (m_bezier)
        return m_bezier->Weight(uIndex, vIndex);
    else if (m_bspline)
        return m_bspline->Weight(uIndex, vIndex);

    return 0.;
}

void ScriptGeomGeneralSplineSurface::throwIfNullSupportSurface() const
{
    if (!m_bezier && !m_bspline)
        throw std::runtime_error("Geom_BezierSurface and Geom_BSplineSurface pointers are null");
}


ScriptGeomBezierSurface::ScriptGeomBezierSurface(const OccHandle<Geom_BezierSurface>& bezier)
    : ScriptGeomGeneralSplineSurface(bezier)
{
}


ScriptGeomBSplineSurface::ScriptGeomBSplineSurface(const OccHandle<Geom_BSplineSurface>& bspline)
    : ScriptGeomGeneralSplineSurface(bspline)
{
}

double ScriptGeomBSplineSurface::uKnot(int uIndex) const
{
    return this->bspline()->UKnot(uIndex);
}

double ScriptGeomBSplineSurface::vKnot(int vIndex) const
{
    return this->bspline()->VKnot(vIndex);
}

int ScriptGeomBSplineSurface::uMultiplicity(int uIndex) const
{
    return this->bspline()->UMultiplicity(uIndex);
}

int ScriptGeomBSplineSurface::vMultiplicity(int vIndex) const
{
    return this->bspline()->UMultiplicity(vIndex);
}

int ScriptGeomBSplineSurface::uKnotIndexFirst() const
{
    return this->bspline()->FirstUKnotIndex();
}

int ScriptGeomBSplineSurface::uKnotIndexLast() const
{
    return this->bspline()->LastUKnotIndex();
}

int ScriptGeomBSplineSurface::vKnotIndexFirst() const
{
    return this->bspline()->FirstVKnotIndex();
}

int ScriptGeomBSplineSurface::vKnotIndexLast() const
{
    return this->bspline()->LastVKnotIndex();
}

int ScriptGeomBSplineSurface::uKnotCount() const
{
    return this->bspline()->NbUKnots();
}

int ScriptGeomBSplineSurface::vKnotCount() const
{
    return this->bspline()->NbVKnots();
}

unsigned int ScriptGeomBSplineSurface::uKnotDistribution() const
{
    return this->bspline()->UKnotDistribution();
}

unsigned int ScriptGeomBSplineSurface::vKnotDistribution() const
{
    return this->bspline()->VKnotDistribution();
}

} // namespace Mayo
