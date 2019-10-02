/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mesh_utils.h"
#include <cmath>

namespace Mayo {

double MeshUtils::triangleSignedVolume(
        const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3)
{
    return p1.Dot(p2.Crossed(p3)) / 6.0f;
}

double MeshUtils::triangleArea(
        const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3)
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

double MeshUtils::triangulationVolume(
        const Handle_Poly_Triangulation& triangulation)
{
    double volume = 0;
    const TColgp_Array1OfPnt& vecNode = triangulation->Nodes();
    // TODO Parallelize computation
    for (const Poly_Triangle& tri : triangulation->Triangles()) {
        int v1, v2, v3;
        tri.Get(v1, v2, v3);
        volume += MeshUtils::triangleSignedVolume(
                    vecNode.Value(v1).Coord(),
                    vecNode.Value(v2).Coord(),
                    vecNode.Value(v3).Coord());
    }

    return std::abs(volume);
}

double MeshUtils::triangulationArea(
        const Handle_Poly_Triangulation& triangulation)
{
    double area = 0;
    const TColgp_Array1OfPnt& vecNode = triangulation->Nodes();
    // TODO Parallelize computation
    for (const Poly_Triangle& tri : triangulation->Triangles()) {
        int v1, v2, v3;
        tri.Get(v1, v2, v3);
        area += MeshUtils::triangleArea(
                    vecNode.Value(v1).Coord(),
                    vecNode.Value(v2).Coord(),
                    vecNode.Value(v3).Coord());
    }

    return area;
}

} // namespace Mayo
