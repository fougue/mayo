/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "math_utils.h"
#include "bnd_utils.h"

#include <Precision.hxx>
#include <algorithm>
#include <cmath>

namespace Mayo {

double MathUtils::mappedValue(double val, double omin, double omax, double nmin, double nmax)
{
    return (((val - omin) * (nmax - nmin)) / (omax - omin)) + nmin;
}

bool MathUtils::isReversedStandardDir(const gp_Dir& n)
{
    for (int i = 0; i < 3; ++i) {
        const double c = n.XYZ().GetData()[i];
        if (c < 0 && (std::abs(c) - 1) < Precision::Confusion())
            return true;
    }

    return false;
}

double MathUtils::planePosition(const gp_Pln& plane)
{
    const gp_Vec vecLoc(plane.Location().XYZ());
    const gp_Vec vecNormal(plane.Axis().Direction());
    return vecLoc.Dot(vecNormal);
}

std::pair<double, double> MathUtils::planeRange(const BndBoxCoords& bbc, const gp_Dir& planeNormal)
{
    const gp_Vec n(MathUtils::isReversedStandardDir(planeNormal) ?
                       planeNormal.Reversed() :
                       planeNormal);
    bool isMaxValid = false;
    bool isMinValid = false;
    double rmax;
    double rmin;
    for (const gp_Pnt& bndPoint : bbc.vertices()) {
        const gp_Vec vec(bndPoint.XYZ());
        const double dot = n.Dot(vec);
        rmax = isMaxValid ? std::max(rmax, dot) : dot;
        rmin = isMinValid ? std::min(rmin, dot) : dot;
        isMaxValid = true;
        isMinValid = true;
    }

    if (isMaxValid && isMinValid)
        return { rmin, rmax };
    return {};
}

} // namespace Mayo
