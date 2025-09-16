/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/occ_handle.h"

#include "script_geom_curve.h"
#include "script_typedefs.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <BRepAdaptor_Surface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>

namespace Mayo {

class ScriptGeomSurface {
    Q_GADGET
    Q_PROPERTY(ScriptGeomSurfaceType type READ type)
    Q_PROPERTY(double uParamFirst READ uParamFirst)
    Q_PROPERTY(double uParamLast READ uParamLast)
    Q_PROPERTY(double vParamFirst READ vParamFirst)
    Q_PROPERTY(double vParamLast READ vParamLast)
    Q_PROPERTY(ScriptGeomContinuity uContinuity READ uContinuity)
    Q_PROPERTY(ScriptGeomContinuity vContinuity READ vContinuity)
    Q_PROPERTY(bool isUClosed READ isUClosed)
    Q_PROPERTY(bool isVClosed READ isVClosed)
    Q_PROPERTY(bool isUPeriodic READ isUPeriodic)
    Q_PROPERTY(bool isVPeriodic READ isVPeriodic)
    Q_PROPERTY(double uPeriod READ uPeriod)
    Q_PROPERTY(double vPeriod READ vPeriod)
    Q_PROPERTY(QVariant_ScriptGeomCylinder cylinder READ cylinder)
    Q_PROPERTY(QVariant_ScriptGeomPlane plane READ plane)
    Q_PROPERTY(QVariant_ScriptGeomCone cone READ cone)
    Q_PROPERTY(QVariant_ScriptGeomSphere sphere READ sphere)
    Q_PROPERTY(QVariant_ScriptGeomTorus torus READ torus)
    Q_PROPERTY(QVariant_ScriptGeomBezierSurface bezier READ bezier)
    Q_PROPERTY(QVariant_ScriptGeomBSplineSurface bspline READ bspline)
    Q_PROPERTY(QVariant_ScriptGeomSurfaceOfLinearExtrusion surfaceOfLinearExtrusion READ surfaceOfLinearExtrusion)
    Q_PROPERTY(QVariant_ScriptGeomSurfaceOfRevolution surfaceOfRevolution READ surfaceOfRevolution)
    Q_PROPERTY(QVariant_ScriptGeomOffsetSurface offsetSurface READ offsetSurface)
public:
    ScriptGeomSurface() = default;
    ScriptGeomSurface(const OccHandle<Geom_Surface>& surface);
    ScriptGeomSurface(const TopoDS_Face& face);

    ScriptGeomSurfaceType type() const { return this->surface().GetType(); }

    double uParamFirst() const { return this->surface().FirstUParameter(); }
    double uParamLast() const { return this->surface().LastUParameter(); }
    double vParamFirst() const { return this->surface().FirstVParameter(); }
    double vParamLast() const { return this->surface().LastVParameter(); }

    ScriptGeomContinuity uContinuity() const { return this->surface().UContinuity(); }
    ScriptGeomContinuity vContinuity() const { return this->surface().VContinuity(); }

    Q_INVOKABLE int uIntervalCount(ScriptGeomContinuity c) const;
    Q_INVOKABLE int vIntervalCount(ScriptGeomContinuity c) const;

    bool isUClosed() const { return this->surface().IsUClosed(); }
    bool isVClosed() const { return this->surface().IsVClosed(); }

    bool isUPeriodic() const { return this->surface().IsUPeriodic(); }
    bool isVPeriodic() const { return this->surface().IsVPeriodic(); }
    double uPeriod() const { return this->surface().UPeriod(); }
    double vPeriod() const { return this->surface().VPeriod(); }

    Q_INVOKABLE QVariant_Coords3D point(double u, double v) const;
    Q_INVOKABLE QVariant_Coords3D dN(double u, double v, int uN, int vN) const;

    QVariant_ScriptGeomCylinder cylinder() const;
    QVariant_ScriptGeomPlane plane() const;
    QVariant_ScriptGeomCone cone() const;
    QVariant_ScriptGeomSphere sphere() const;
    QVariant_ScriptGeomTorus torus() const;
    QVariant_ScriptGeomBezierSurface bezier() const;
    QVariant_ScriptGeomBSplineSurface bspline() const;
    QVariant_ScriptGeomSurfaceOfLinearExtrusion surfaceOfLinearExtrusion() const;
    QVariant_ScriptGeomSurfaceOfRevolution surfaceOfRevolution() const;
    QVariant_ScriptGeomOffsetSurface offsetSurface() const;

private:
    const Adaptor3d_Surface& surface() const;
    const OccHandle<Geom_Surface>& geomSurface() const;
    const gp_Trsf& trsf() const;
    GeomAdaptor_Surface m_geomSurface;
    BRepAdaptor_Surface m_brepSurface;
};


class ScriptGeomCylinder {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx3 position READ position)
    Q_PROPERTY(double radius READ radius)
public:
    ScriptGeomCylinder() = default;
    ScriptGeomCylinder(const gp_Cylinder& cyl);

    QVariant_ScriptGeomAx3 position() const;
    double radius() const { return m_cyl.Radius(); }

private:
    gp_Cylinder m_cyl;
};


class ScriptGeomPlane {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx3 position READ position)
public:
    ScriptGeomPlane() = default;
    ScriptGeomPlane(const gp_Pln& pln);

    QVariant_ScriptGeomAx3 position() const;

private:
    gp_Pln m_pln;
};


class ScriptGeomCone {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx3 position READ position)
    Q_PROPERTY(QVariant_Coords3D apex READ apex)
    Q_PROPERTY(double refRadius READ refRadius)
    Q_PROPERTY(double semiAngle READ semiAngle)
public:
    ScriptGeomCone() = default;
    ScriptGeomCone(const gp_Cone& cone);

    QVariant_ScriptGeomAx3 position() const;
    QVariant_Coords3D apex() const;
    double refRadius() const { return m_cone.RefRadius(); }
    double semiAngle() const { return m_cone.SemiAngle(); }

private:
    gp_Cone m_cone;
};


class ScriptGeomSphere {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx3 position READ position)
    Q_PROPERTY(double radius READ radius)
public:
    ScriptGeomSphere() = default;
    ScriptGeomSphere(const gp_Sphere& sphere);

    QVariant_ScriptGeomAx3 position() const;
    double radius() const { return m_sphere.Radius(); }

private:
    gp_Sphere m_sphere;
};


class ScriptGeomTorus {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomAx3 position READ position)
    Q_PROPERTY(double majorRadius READ majorRadius)
    Q_PROPERTY(double minorRadius READ minorRadius)
public:
    ScriptGeomTorus() = default;
    ScriptGeomTorus(const gp_Torus& torus);

    QVariant_ScriptGeomAx3 position() const;
    double majorRadius() const { return m_torus.MajorRadius(); }
    double minorRadius() const { return m_torus.MinorRadius(); }

private:
    gp_Torus m_torus;
};


class ScriptGeomGeneralSplineSurface {
    Q_GADGET
    Q_PROPERTY(int uDegree READ uDegree)
    Q_PROPERTY(int vDegree READ vDegree)
    Q_PROPERTY(int uPoleCount READ uPoleCount)
    Q_PROPERTY(int vPoleCount READ vPoleCount)
    Q_PROPERTY(bool isURational READ isURational)
    Q_PROPERTY(bool isVRational READ isVRational)
public:
    ScriptGeomGeneralSplineSurface() = default;

    int uDegree() const;
    int vDegree() const;
    int uPoleCount() const;
    int vPoleCount() const;
    bool isURational() const;
    bool isVRational() const;

    Q_INVOKABLE QVariant_Coords3D pole(int uIndex, int vIndex) const;
    Q_INVOKABLE double weight(int uIndex, int vIndex) const;

protected:
    ScriptGeomGeneralSplineSurface(const OccHandle<Geom_BezierSurface>& bezier);
    ScriptGeomGeneralSplineSurface(const OccHandle<Geom_BSplineSurface>& bspline);

    const OccHandle<Geom_BezierSurface>& bezier() const { return m_bezier; }
    const OccHandle<Geom_BSplineSurface>& bspline() const { return m_bspline; }

    void throwIfNullSupportSurface() const;

private:
    OccHandle<Geom_BezierSurface> m_bezier;
    OccHandle<Geom_BSplineSurface> m_bspline;
};


class ScriptGeomBezierSurface : public ScriptGeomGeneralSplineSurface {
    Q_GADGET
public:
    ScriptGeomBezierSurface() = default;
    ScriptGeomBezierSurface(const OccHandle<Geom_BezierSurface>& bezier);
};


class ScriptGeomBSplineSurface : public ScriptGeomGeneralSplineSurface {
    Q_GADGET
    Q_PROPERTY(int uKnotIndexFirst READ uKnotIndexFirst)
    Q_PROPERTY(int uKnotIndexLast READ uKnotIndexLast)
    Q_PROPERTY(int vKnotIndexFirst READ vKnotIndexFirst)
    Q_PROPERTY(int vKnotIndexLast READ vKnotIndexLast)
    Q_PROPERTY(int uKnotCount READ uKnotCount)
    Q_PROPERTY(int vKnotCount READ vKnotCount)
    Q_PROPERTY(ScriptGeomBSplineKnotDistribution uKnotDistribution READ uKnotDistribution)
    Q_PROPERTY(ScriptGeomBSplineKnotDistribution vKnotDistribution READ vKnotDistribution)
public:
    ScriptGeomBSplineSurface() = default;
    ScriptGeomBSplineSurface(const OccHandle<Geom_BSplineSurface>& bspline);

    Q_INVOKABLE double uKnot(int uIndex) const;
    Q_INVOKABLE double vKnot(int vIndex) const;

    Q_INVOKABLE int uMultiplicity(int uIndex) const;
    Q_INVOKABLE int vMultiplicity(int vIndex) const;

    int uKnotIndexFirst() const;
    int uKnotIndexLast() const;

    int vKnotIndexFirst() const;
    int vKnotIndexLast() const;

    int uKnotCount() const;
    int vKnotCount() const;

    ScriptGeomBSplineKnotDistribution uKnotDistribution() const;
    ScriptGeomBSplineKnotDistribution vKnotDistribution() const;
};


template<typename GeomSurfaceType>
class GeomSurfaceHelper {
public:
    GeomSurfaceHelper() = default;
    GeomSurfaceHelper(const OccHandle<GeomSurfaceType>& surface, const gp_Trsf& trsf)
        : m_surface(surface), m_trsf(trsf)
    {}

    const gp_Trsf& trsf() const { return m_trsf; }
    const OccHandle<GeomSurfaceType>& inputSurf() const { return m_surface; }
    const OccHandle<GeomSurfaceType>& trsfSurf() const {
        if (m_trsf.Form() == gp_Identity)
            return m_surface;

        if (!m_surfaceTransformed) {
            OccHandle<Geom_Geometry> geom = m_surface->Transformed(m_trsf);
            m_surfaceTransformed = OccHandle<GeomSurfaceType>::DownCast(geom);
        }

        return m_surfaceTransformed;
    }

    bool valid() const { return !m_surface.IsNull(); }

private:
    OccHandle<GeomSurfaceType> m_surface;
    mutable OccHandle<GeomSurfaceType> m_surfaceTransformed;
    gp_Trsf m_trsf;
};

class ScriptGeomSurfaceOfLinearExtrusion {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomCurve basisCurve READ basisCurve)
    Q_PROPERTY(QVariant_Coords3D direction READ direction)
public:
    ScriptGeomSurfaceOfLinearExtrusion() = default;
    ScriptGeomSurfaceOfLinearExtrusion(
        const OccHandle<Geom_SurfaceOfLinearExtrusion>& surfExtrusion, const gp_Trsf& trsf
    );

    QVariant_ScriptGeomCurve basisCurve() const;
    QVariant_Coords3D direction() const;

private:
    GeomSurfaceHelper<Geom_SurfaceOfLinearExtrusion> m_surface;
};


class ScriptGeomSurfaceOfRevolution {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomCurve basisCurve READ basisCurve)
    Q_PROPERTY(QVariant_ScriptGeomAx1 axis READ axis)
    Q_PROPERTY(QVariant_Coords3D direction READ direction)
public:
    ScriptGeomSurfaceOfRevolution() = default;
    ScriptGeomSurfaceOfRevolution(
        const OccHandle<Geom_SurfaceOfRevolution>& surfRevolution, const gp_Trsf& trsf
    );

    QVariant_ScriptGeomCurve basisCurve() const;
    QVariant_ScriptGeomAx1 axis() const;
    QVariant_Coords3D direction() const;

private:
    GeomSurfaceHelper<Geom_SurfaceOfRevolution> m_surface;
};

class ScriptGeomOffsetSurface {
    Q_GADGET
    Q_PROPERTY(QVariant_ScriptGeomSurface basisSurface READ basisSurface)
    Q_PROPERTY(double value READ value)
public:
    ScriptGeomOffsetSurface() = default;
    ScriptGeomOffsetSurface(const OccHandle<Geom_OffsetSurface>& offset, const gp_Trsf& trsf);

    QVariant_ScriptGeomSurface basisSurface() const;
    double value() const;

private:
    GeomSurfaceHelper<Geom_OffsetSurface> m_surface;
};

} // namespace Mayo

Q_DECLARE_METATYPE(Mayo::ScriptGeomSurface)

Q_DECLARE_METATYPE(Mayo::ScriptGeomCylinder)
Q_DECLARE_METATYPE(Mayo::ScriptGeomPlane)
Q_DECLARE_METATYPE(Mayo::ScriptGeomCone)
Q_DECLARE_METATYPE(Mayo::ScriptGeomSphere)
Q_DECLARE_METATYPE(Mayo::ScriptGeomTorus)
Q_DECLARE_METATYPE(Mayo::ScriptGeomBezierSurface)
Q_DECLARE_METATYPE(Mayo::ScriptGeomBSplineSurface)
Q_DECLARE_METATYPE(Mayo::ScriptGeomSurfaceOfLinearExtrusion)
Q_DECLARE_METATYPE(Mayo::ScriptGeomSurfaceOfRevolution)
Q_DECLARE_METATYPE(Mayo::ScriptGeomOffsetSurface)
