/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Poly_Triangulation.hxx>
class gp_XYZ;

namespace occ {

struct MeshUtils {
    static double triangleSignedVolume(
            const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);
    static double triangleArea(
            const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);

    static double triangulationVolume(const Handle_Poly_Triangulation& triangulation);
    static double triangulationArea(const Handle_Poly_Triangulation& triangulation);
};

} // namespace occ
