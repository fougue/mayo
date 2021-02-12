/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <Poly_Triangulation.hxx>
class gp_XYZ;

namespace Mayo {

struct MeshUtils {
    static double triangleSignedVolume(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);
    static double triangleArea(const gp_XYZ& p1, const gp_XYZ& p2, const gp_XYZ& p3);

    static double triangulationVolume(const Handle_Poly_Triangulation& triangulation);
    static double triangulationArea(const Handle_Poly_Triangulation& triangulation);

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
