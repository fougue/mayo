/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "geom_utils.h"
#include "math_utils.h"

#include <Adaptor3d_Curve.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Version.hxx>
#include <TopLoc_Location.hxx>

#include <algorithm>
#include <cmath>

namespace Mayo {

double GeomUtils::normalizedU(const Adaptor3d_Curve& curve, double u)
{
    const double uFirst = curve.FirstParameter();
    const double uLast = curve.LastParameter();
    const std::pair<double, double> uRange = std::minmax(uFirst, uLast);
    return MathUtils::mappedValue(u, uRange.first, uRange.second, 0, 1);
}

gp_Pnt GeomUtils::d0(const Adaptor3d_Curve& curve, double u)
{
    gp_Pnt pnt;
    curve.D0(u, pnt);
    return pnt;
}

gp_Vec GeomUtils::d1(const Adaptor3d_Curve& curve, double u)
{
    gp_Pnt pnt;
    gp_Vec vec;
    curve.D1(u, pnt, vec);
    return vec;
}

std::pair<gp_Pnt, gp_Vec> GeomUtils::d0d1(const Adaptor3d_Curve& curve, double u)
{
    gp_Pnt pnt;
    gp_Vec vec;
    curve.D1(u, pnt, vec);
    return { pnt, vec };
}

bool GeomUtils::hasScaling(const gp_Trsf& trsf)
{
#if OCC_VERSION_HEX >= 0x070600
    const double scalePrec = TopLoc_Location::ScalePrec();
#else
    constexpr double scalePrec = 1.e-14;
#endif
    // This test comes from implementation of TopoDS_Shape::Location() function
    return std::abs(std::abs(trsf.ScaleFactor()) - 1.) > scalePrec || trsf.IsNegative();
}

} // namespace Mayo::GeomUtils
