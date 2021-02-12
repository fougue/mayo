/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <utility>

namespace Mayo {

struct BndBoxCoords;

struct MathUtils {
    static double mappedValue(double val, double omin, double omax, double nmin, double nmax);

    static bool isReversedStandardDir(const gp_Dir& n);
    static double planePosition(const gp_Pln& plane);
    static std::pair<double, double> planeRange(const BndBoxCoords& bbc, const gp_Dir& planeNormal);

    template<typename T, typename U> static T lerp(T a, T b, U t);
};


// --
// -- Implementation
// --

template<typename T, typename U> T MathUtils::lerp(T a, T b, U t)
{
    // TODO Call std::lerp() when available(C++20)
    return T(a * (1 - t) + b * t);
}

} // namespace Mayo
