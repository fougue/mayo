#pragma once

#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <utility>

namespace Mayo {

struct BndBoxCoords;

struct MathUtils {
    static double mappedValue(
            double val, double omin, double omax, double nmin, double nmax);

    static bool isReversedStandardDir(const gp_Dir& n);
    static double planePosition(const gp_Pln& plane);
    static std::pair<double, double> planeRange(
            const BndBoxCoords& bbc, const gp_Dir& planeNormal);
};

} // namespace Mayo
