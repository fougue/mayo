/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "mesh_utils.h"
#include <cmath>

namespace occ {

double MeshUtils::triangleSignedVolume(
        const gp_XYZ &p1, const gp_XYZ &p2, const gp_XYZ &p3)
{
    return p1.Dot(p2.Crossed(p3)) / 6.0f;
}

double MeshUtils::triangleArea(
        const gp_XYZ &p1, const gp_XYZ &p2, const gp_XYZ &p3)
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
        const Handle_Poly_Triangulation &triangulation)
{
    double volume = 0;
    const TColgp_Array1OfPnt& vecNode = triangulation->Nodes();
    const Poly_Array1OfTriangle& vecTriangle = triangulation->Triangles();
    // TODO Parallelize computation
    for (int i = 1; i < vecTriangle.Size(); ++i) {
        const Poly_Triangle& tri = vecTriangle.Value(i);
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
        const Handle_Poly_Triangulation &triangulation)
{
    double area = 0;
    const TColgp_Array1OfPnt& vecNode = triangulation->Nodes();
    const Poly_Array1OfTriangle& vecTriangle = triangulation->Triangles();
    // TODO Parallelize computation
    for (int i = 1; i < vecTriangle.Size(); ++i) {
        const Poly_Triangle& tri = vecTriangle.Value(i);
        int v1, v2, v3;
        tri.Get(v1, v2, v3);
        area += MeshUtils::triangleArea(
                    vecNode.Value(v1).Coord(),
                    vecNode.Value(v2).Coord(),
                    vecNode.Value(v3).Coord());
    }
    return area;
}

} // namespace occ
