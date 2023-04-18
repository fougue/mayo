/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Poly_Triangulation.hxx>
#include <Standard_Version.hxx>
class gp_XYZ;

namespace Mayo {

// Provides helper functions for mesh and triangle objects
struct MeshUtils {
    static double triangleSignedVolume(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);
    static double triangleArea(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);

    static double triangulationVolume(const Handle_Poly_Triangulation& triangulation);
    static double triangulationArea(const Handle_Poly_Triangulation& triangulation);

#if OCC_VERSION_HEX >= 0x070600
    using Poly_Triangulation_NormalType = gp_Vec3f;
#else
    using Poly_Triangulation_NormalType = gp_Vec;
#endif

    static void setNode(const Handle_Poly_Triangulation& triangulation, int index, const gp_Pnt& pnt);
    static void setTriangle(const Handle_Poly_Triangulation& triangulation, int index, const Poly_Triangle& triangle);
    static void setNormal(const Handle_Poly_Triangulation& triangulation, int index, const Poly_Triangulation_NormalType& n);
    static void allocateNormals(const Handle_Poly_Triangulation& triangulation);

    static const Poly_Array1OfTriangle& triangles(const Handle_Poly_Triangulation& triangulation) {
#if OCC_VERSION_HEX < 0x070600
        return triangulation->Triangles();
#else
        // Note: Poly_Triangulation::Triangles() was deprecated starting from OpenCascade v7.6.0
        return triangulation->InternalTriangles();
#endif
    }

    enum class Orientation {
        Unknown,
        Clockwise,
        CounterClockwise
    };

    class AdaptorPolyline2d {
    public:
        virtual gp_Pnt2d pointAt(int index) const = 0;
        virtual int pointCount() const = 0;
        virtual bool empty() const { return this->pointCount() <= 0; }
    };

    class AdaptorPolyline3d {
    public:
        virtual const gp_Pnt& pointAt(int i) const = 0;
        virtual int pointCount() const = 0;
        virtual int empty() const { return this->pointCount() <= 0; }
    };

    static Orientation orientation(const AdaptorPolyline2d& polyline);
    static gp_Vec directionAt(const AdaptorPolyline3d& polyline, int i);
};

} // namespace Mayo
