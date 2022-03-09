/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <algorithm>
#include <cmath>
#include <utility>

namespace Mayo {

struct BndBoxCoords;

// Provides helper functions for mathematics purpose
namespace MathUtils {

// Returns the value 'val' which is in range [omin..omax] to the corresponding value in range [nmin..nmax]
double mappedValue(double val, double omin, double omax, double nmin, double nmax);

// Is 'n' a standard direction being reversed(ie -X, -Y or -Z) ?
bool isReversedStandardDir(const gp_Dir& n);

// Returns linear position of a plane along its normal direction
double planePosition(const gp_Pln& plane);

// Returns the min/max range of linear positions along direction 'planeNormal' within box 'bbc'
std::pair<double, double> planeRange(const BndBoxCoords& bbc, const gp_Dir& planeNormal);

// Returns a + t(b − a)
template<typename T, typename U> static T lerp(T a, T b, U t);

// Returns true if the absolute value of 'f' is within 0.00001f of 0.0
inline bool fuzzyIsNull(float f) { return std::abs(f) <= 0.00001f; }

// Returns true if the absolute value of 'd' is within 0.000000000001 of 0.0
inline bool fuzzyIsNull(double d) { return std::abs(d) <= 0.000000000001; }

// Compares the floating point value 'f1' and 'f2' and returns true if they are considered equal, otherwise false.
// Note that comparing values where either 'f1' or 'f2' is 0.0 will not work, nor does comparing values where one of
// the values is NaN or infinity. If one of the values is always 0.0, use fuzzyIsNull() instead.
// If one of the values is likely to be 0.0, one solution is to add 1.0 to both values
inline bool fuzzyEqual(float f1, float f2) {
    return (std::abs(f1 - f2) * 100000.f <= std::min(std::abs(f1), std::abs(f2)));
}

// Same as fuzzyEqual(float, float) but for double-precision values
inline bool fuzzyEqual(double d1, double d2) {
    return (std::abs(d1 - d2) * 1000000000000. <= std::min(std::abs(d1), std::abs(d2)));
}



// --
// -- Implementation
// --

template<typename T, typename U> T lerp(T a, T b, U t)
{
#ifdef __cpp_lib_interpolate
    return std::lerp(a, b, t);
#else
    return T(a * (1 - t) + b * t);
#endif
}

} // namespace MathUtils
} // namespace Mayo
