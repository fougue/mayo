/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/occ_handle.h"

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include <BRepAdaptor_Surface.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierSurface.hxx>
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
    Q_PROPERTY(unsigned type READ type)
    Q_PROPERTY(double uParamFirst READ uParamFirst)
    Q_PROPERTY(double uParamLast READ uParamLast)
    Q_PROPERTY(double vParamFirst READ vParamFirst)
    Q_PROPERTY(double vParamLast READ vParamLast)
    Q_PROPERTY(unsigned uContinuity READ uContinuity)
    Q_PROPERTY(unsigned vContinuity READ vContinuity)
    Q_PROPERTY(bool isUClosed READ isUClosed)
    Q_PROPERTY(bool isVClosed READ isVClosed)
    Q_PROPERTY(bool isUPeriodic READ isUPeriodic)
    Q_PROPERTY(bool isVPeriodic READ isVPeriodic)
    Q_PROPERTY(double uPeriod READ uPeriod)
    Q_PROPERTY(double vPeriod READ vPeriod)
    Q_PROPERTY(QVariant cylinder READ cylinder)
    Q_PROPERTY(QVariant plane READ plane)
    Q_PROPERTY(QVariant cone READ cone)
    Q_PROPERTY(QVariant sphere READ sphere)
    Q_PROPERTY(QVariant torus READ torus)
    Q_PROPERTY(QVariant bezier READ bezier)
    Q_PROPERTY(QVariant bspline READ bspline)
public:
    ScriptGeomSurface() = default;
    ScriptGeomSurface(const TopoDS_Face& face);

    unsigned type() const { return m_surface.GetType(); }

    double uParamFirst() const { return m_surface.FirstUParameter(); }
    double uParamLast() const { return m_surface.LastUParameter(); }
    double vParamFirst() const { return m_surface.FirstVParameter(); }
    double vParamLast() const { return m_surface.LastVParameter(); }

    unsigned uContinuity() const { return m_surface.UContinuity(); }
    unsigned vContinuity() const { return m_surface.VContinuity(); }

    Q_INVOKABLE int uIntervalCount(unsigned continuity) const;
    Q_INVOKABLE int vIntervalCount(unsigned continuity) const;

    bool isUClosed() const { return m_surface.IsUClosed(); }
    bool isVClosed() const { return m_surface.IsVClosed(); }

    bool isUPeriodic() const { return m_surface.IsUPeriodic(); }
    bool isVPeriodic() const { return m_surface.IsVPeriodic(); }
    double uPeriod() const { return m_surface.UPeriod(); }
    double vPeriod() const { return m_surface.VPeriod(); }

    Q_INVOKABLE QVariant point(double u, double v) const;              // ->{x, y, z}
    Q_INVOKABLE QVariant dN(double u, double v, int uN, int vN) const; // ->{x, y, z}

    QVariant cylinder() const; // ->ScriptGeomCylinder
    QVariant plane() const;    // ->ScriptGeomPlane
    QVariant cone() const;     // ->ScriptGeomCone
    QVariant sphere() const;   // ->ScriptGeomSphere
    QVariant torus() const;    // ->ScriptGeomTorus
    QVariant bezier() const;   // ->ScriptGeomBezierSurface
    QVariant bspline() const;  // ->ScriptGeomBSplineSurface
    QVariant surfaceOfLinearExtrusion() const; // ->ScriptGeomSurfaceOfLinearExtrusion
    QVariant surfaceOfRevolution() const;      // ->ScriptGeomSurfaceOfRevolution

private:
    BRepAdaptor_Surface m_surface;
};


class ScriptGeomCylinder {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double radius READ radius)
public:
    ScriptGeomCylinder() = default;
    ScriptGeomCylinder(const gp_Cylinder& cyl);

    QVariant position() const; // ->ScriptGeomAx3
    double radius() const { return m_cyl.Radius(); }

private:
    gp_Cylinder m_cyl;
};


class ScriptGeomPlane {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
public:
    ScriptGeomPlane() = default;
    ScriptGeomPlane(const gp_Pln& pln);

    QVariant position() const; // ->ScriptGeomAx3

private:
    gp_Pln m_pln;
};


class ScriptGeomCone {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(QVariant apex READ apex)
    Q_PROPERTY(double refRadius READ refRadius)
    Q_PROPERTY(double semiAngle READ semiAngle)
public:
    ScriptGeomCone() = default;
    ScriptGeomCone(const gp_Cone& cone);

    QVariant position() const; // ->ScriptGeomAx3
    QVariant apex() const;     // ->{x, y, z}
    double refRadius() const { return m_cone.RefRadius(); }
    double semiAngle() const { return m_cone.SemiAngle(); }

private:
    gp_Cone m_cone;
};


class ScriptGeomSphere {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double radius READ radius)
public:
    ScriptGeomSphere() = default;
    ScriptGeomSphere(const gp_Sphere& sphere);

    QVariant position() const; // ->ScriptGeomAx3
    double radius() const { return m_sphere.Radius(); }

private:
    gp_Sphere m_sphere;
};


class ScriptGeomTorus {
    Q_GADGET
    Q_PROPERTY(QVariant position READ position)
    Q_PROPERTY(double majorRadius READ majorRadius)
    Q_PROPERTY(double minorRadius READ minorRadius)
public:
    ScriptGeomTorus() = default;
    ScriptGeomTorus(const gp_Torus& torus);

    QVariant position() const; // ->ScriptGeomAx3
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

    Q_INVOKABLE QVariant pole(int uIndex, int vIndex) const;  // ->{x, y, z}
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
    Q_PROPERTY(unsigned uKnotDistribution READ uKnotDistribution)
    Q_PROPERTY(unsigned vKnotDistribution READ vKnotDistribution)
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

    unsigned uKnotDistribution() const;
    unsigned vKnotDistribution() const;
};


class ScriptGeomSurfaceOfLinearExtrusion {
    Q_GADGET
    Q_PROPERTY(QVariant basisCurve READ basisCurve)
    Q_PROPERTY(QVariant direction READ direction)
public:
    ScriptGeomSurfaceOfLinearExtrusion() = default;
    ScriptGeomSurfaceOfLinearExtrusion(const OccHandle<Geom_SurfaceOfLinearExtrusion>& surfExtrusion);

    QVariant basisCurve() const; // ->ScriptGeomCurve
    QVariant direction() const;  // ->{x, y, z}

private:
    OccHandle<Geom_SurfaceOfLinearExtrusion> m_surfExtrusion;
};


class ScriptGeomSurfaceOfRevolution {
    Q_GADGET
    Q_PROPERTY(QVariant axis READ axis)
    Q_PROPERTY(QVariant basisCurve READ basisCurve)
    Q_PROPERTY(QVariant direction READ direction)
public:
    ScriptGeomSurfaceOfRevolution() = default;
    ScriptGeomSurfaceOfRevolution(const OccHandle<Geom_SurfaceOfRevolution>& surfRevolution);

    QVariant axis() const;       // ->ScriptGeomAx1
    QVariant basisCurve() const; // ->ScriptGeomCurve
    QVariant direction() const;  // ->{x, y, z}

private:
    OccHandle<Geom_SurfaceOfRevolution> m_surfRevolution;
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
