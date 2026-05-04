/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "math_utils.h"
#include "bnd_utils.h"

#include <Precision.hxx>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Mayo::MathUtils {

bool isReversedStandardDir(const gp_Dir& n)
{
    auto isUnitCoord = [](double v) -> bool {
        return v < 0 && std::abs(v + 1.) < Precision::Confusion();
    };
    return isUnitCoord(n.X()) || isUnitCoord(n.Y()) || isUnitCoord(n.Z());
}

double planePosition(const gp_Pln& plane)
{
    const gp_Vec vecLoc(plane.Location().XYZ());
    const gp_Vec vecNormal(plane.Axis().Direction());
    return vecLoc.Dot(vecNormal);
}

std::pair<double, double> planeRange(const BndBoxCoords& bbc, const gp_Dir& planeNormal)
{
    const gp_Vec n(isReversedStandardDir(planeNormal) ? planeNormal.Reversed() : planeNormal);
    bool isMaxValid = false;
    bool isMinValid = false;
    double rmax = std::numeric_limits<double>::lowest();
    double rmin = std::numeric_limits<double>::max();
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

int intRound(double v) noexcept
{
    if (std::isnan(v))
        return 0;

    auto lv = std::lround(v);
    if constexpr (sizeof(int) != sizeof(long))
        lv = std::clamp(lv, static_cast<long>(INT_MIN), static_cast<long>(INT_MAX));

    return static_cast<int>(lv);
}

} // namespace Mayo::MathUtils
