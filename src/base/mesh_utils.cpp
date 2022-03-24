/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mesh_utils.h"
#include "math_utils.h"
#include <Standard_Version.hxx>
#include <cmath>

namespace Mayo {

double MeshUtils::triangleSignedVolume(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3)
{
    return p1.Dot(p2.Crossed(p3)) / 6.0f;
}

double MeshUtils::triangleArea(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3)
{
    const double ax = p2.X() - p1.X();
    const double ay = p2.Y() - p1.Y();
    const double az = p2.Z() - p1.Z();
    const double bx = p3.X() - p1.X();
    const double by = p3.Y() - p1.Y();
    const double bz = p3.Z() - p1.Z();
    const double cx = ay*bz - az*by;
    const double cy = az*bx - ax*bz;
    const double cz = ax*by - ay*bx;
    return 0.5 * std::sqrt(cx*cx + cy*cy + cz*cz);
}

double MeshUtils::triangulationVolume(const Handle_Poly_Triangulation& triangulation)
{
    if (!triangulation)
        return 0;

    double volume = 0;
    // TODO Parallelize computation
    for (const Poly_Triangle& tri : triangulation->Triangles()) {
        int v1, v2, v3;
        tri.Get(v1, v2, v3);
        volume += MeshUtils::triangleSignedVolume(
                    triangulation->Node(v1).Coord(),
                    triangulation->Node(v2).Coord(),
                    triangulation->Node(v3).Coord());
    }

    return std::abs(volume);
}

double MeshUtils::triangulationArea(const Handle_Poly_Triangulation& triangulation)
{
    if (!triangulation)
        return 0;

    double area = 0;
    // TODO Parallelize computation
    for (const Poly_Triangle& tri : triangulation->Triangles()) {
        int v1, v2, v3;
        tri.Get(v1, v2, v3);
        area += MeshUtils::triangleArea(
                    triangulation->Node(v1).Coord(),
                    triangulation->Node(v2).Coord(),
                    triangulation->Node(v3).Coord());
    }

    return area;
}

void MeshUtils::setNode(const Handle_Poly_Triangulation& triangulation, int index, const gp_Pnt& pnt)
{
#if OCC_VERSION_HEX >= 0x070600
    triangulation->SetNode(index, pnt);
#else
    triangulation->ChangeNode(index) = pnt;
#endif
}

void MeshUtils::setTriangle(const Handle_Poly_Triangulation& triangulation, int index, const Poly_Triangle& triangle)
{
#if OCC_VERSION_HEX >= 0x070600
    triangulation->SetTriangle(index, triangle);
#else
    triangulation->ChangeTriangle(index) = triangle;
#endif
}

void MeshUtils::setNormal(const Handle_Poly_Triangulation& triangulation, int index, const Poly_Triangulation_NormalType& n)
{
#if OCC_VERSION_HEX >= 0x070600
    triangulation->SetNormal(index, n);
#else
    TShort_Array1OfShortReal& normals = triangulation->ChangeNormals();
    normals.ChangeValue(index * 3 - 2) = n.X();
    normals.ChangeValue(index * 3 - 1) = n.Y();
    normals.ChangeValue(index * 3)     = n.Z();
#endif
}

void MeshUtils::allocateNormals(const Handle_Poly_Triangulation& triangulation)
{
#if OCC_VERSION_HEX >= 0x070600
    triangulation->AddNormals();
#else
    auto normalCoords = new TShort_HArray1OfShortReal(1, 3 * triangulation->NbNodes());
    triangulation->SetNormals(normalCoords);
#endif
}

// Adapted from http://cs.smith.edu/~jorourke/Code/polyorient.C
MeshUtils::Orientation MeshUtils::orientation(const AdaptorPolyline2d& polyline)
{
    const int pntCount = polyline.pointCount();
    if (pntCount < 2)
        return Orientation::Unknown;

    gp_Pnt2d pntExtreme = polyline.pointAt(0);
    int indexPntExtreme = 0;
    for (int i = 1; i < pntCount; ++i) {
        const gp_Pnt2d pnt = polyline.pointAt(i);
        if (pnt.Y() < pntExtreme.Y()
                || (MathUtils::fuzzyEqual(pnt.Y(), pntExtreme.Y()) && (pnt.X() > pntExtreme.X())))
        {
            pntExtreme = pnt;
            indexPntExtreme = i;
        }
    }

    const gp_Pnt2d beforeExtremePnt = polyline.pointAt((indexPntExtreme + (pntCount - 1)) % pntCount);
    const gp_Pnt2d afterExtremePnt = polyline.pointAt((indexPntExtreme + 1) % pntCount);
    const gp_Pnt2d& a = beforeExtremePnt;
    const gp_Pnt2d& b = pntExtreme;
    const gp_Pnt2d& c = afterExtremePnt;
    const double triangleArea =
            a.X() * b.Y() - a.Y() * b.X()
            + a.Y() * c.X() - a.X() * c.Y()
            + b.Y() * c.X() - c.X() * b.Y();

    auto fnQualifyArea = [](double area) {
        if (area > 0)
            return Orientation::CounterClockwise;
        else if (area < 0)
            return Orientation::Clockwise;
        else
            return Orientation::Unknown;
    };

    const Orientation orientation = fnQualifyArea(triangleArea);
    if (orientation != Orientation::Unknown) {
        return orientation;
    }
    else {
        double polylineArea = 0.;
        for (int i = 0; i < pntCount; ++i) {
            const gp_Pnt2d pntBefore = polyline.pointAt((i + (pntCount - 1)) % pntCount);
            const gp_Pnt2d pntCurrent = polyline.pointAt(i);
            const gp_Pnt2d pntAfter = polyline.pointAt((i + 1) % pntCount);
            polylineArea += pntCurrent.X() * (pntAfter.Y() - pntBefore.Y());
        }

        return fnQualifyArea(polylineArea);
    }
}

gp_Vec MeshUtils::directionAt(const AdaptorPolyline3d& polyline, int i)
{
    const int pntCount = polyline.pointCount();
    if (pntCount > 1) {
        const gp_Pnt& pnt = polyline.pointAt(i);
        const int indexLastPos = pntCount - 1;
        if (i < indexLastPos) {
            const gp_Pnt& nextPnt = polyline.pointAt(i + 1);
            return gp_Vec(pnt, nextPnt);
        }
        else if (i == indexLastPos) {
            const gp_Pnt& prevPnt = polyline.pointAt(i - 1);
            return gp_Vec(prevPnt, pnt);
        }
    }

    return gp_Vec();
}

} // namespace Mayo
