/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "brep_meshing.h"

#include "../base/bnd_utils.h"
#include "../base/brep_utils.h"
#include "../base/occ_brep_mesh_parameters.h"
#include "../base/unit_system.h"
#include "../base/tkernel_utils.h"
#include "../base/xcaf.h"

#include <BRepBndLib.hxx>

#include <algorithm>

namespace Mayo {

const BRepMeshingOptions::QualityCoefficients& BRepMeshingOptions::defaultQualityCoefficients()
{
    static const QualityCoefficients coeffs = {
        { 8, 4 },       // VeryCoarse
        { 4, 2 },       // Coarse
        { 1, 1 },       // Normal
        { 1/4., 1/2. }, // Precise
        { 1/8., 1/4. }, // VeryPrecise
    };
    return coeffs;
}

const BRepMeshingOptions::Coefficients& BRepMeshingOptions::coefficients(Quality quality) const
{
    static const Coefficients neutral = {1, 1};
    const QualityCoefficients& coeffs =
        this->qualityCoefficients ? *this->qualityCoefficients : defaultQualityCoefficients();
    switch (quality) {
    case Quality::VeryCoarse: return coeffs.veryCoarse;
    case Quality::Coarse: return coeffs.coarse;
    case Quality::Normal: return coeffs.normal;
    case Quality::Precise: return coeffs.precise;
    case Quality::VeryPrecise: return coeffs.veryPrecise;
    case Quality::UserDefined: return neutral;
    }

    return neutral;
}

QuantityLength BRepMeshingUtils::chordalDeflectionEstimate(const TopoDS_Shape& shape)
{
    constexpr QuantityLength defaultDeviation = 1 * Quantity_Millimeter;
    // Excerpted from Prs3d::GetDeflection(...)
    Bnd_Box bndBox;
    BRepBndLib::Add(shape, bndBox, false/*!useTriangulation*/);
    if (bndBox.IsVoid())
        return defaultDeviation;

    if (bndBox.IsOpen()) {
        if (!bndBox.HasFinitePart())
            return defaultDeviation;

        bndBox = bndBox.FinitePart();
    }

    const auto coords = BndBoxCoords::get(bndBox);
    const gp_XYZ diag = coords.maxVertex().XYZ() - coords.minVertex().XYZ();
    const double diagMaxComp = std::max({ diag.X(), diag.Y(), diag.Z() });
    constexpr double relativeDeviation = 0.001; // 0.1%
    return (4 * diagMaxComp * relativeDeviation) * Quantity_Millimeter;
}

void BRepMeshingUtils::compute(
        const TopoDS_Shape& shape, const BRepMeshingOptions& options, TaskProgress* progress
    )
{
    OccBRepMeshParameters params;
    params.InParallel = true;
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 5, 0)
    params.AllowQualityDecrease = true;
#endif
    if (options.quality == BRepMeshingOptions::Quality::UserDefined) {
        params.Deflection = UnitSystem::millimeters(options.customChordalDeflection);
        params.Angle = UnitSystem::radians(options.customAngularDeflection);
        params.Relative = options.customRelative;
    }
    else {
        const BRepMeshingOptions::Coefficients& coeffs = options.coefficients(options.quality);
        params.Deflection = UnitSystem::millimeters(coeffs.chordalDeflection * chordalDeflectionEstimate(shape));
        params.Angle = UnitSystem::radians(coeffs.angularDeflection * (20 * Quantity_Degree));
    }

    // NOTE BRepMesh_IncrementalMesh may go to infinite loop on degenerated cases(eg compound shape
    //      containing one vertex) because of resulting deflection that would be very small and
    //      not reachable regarding convergence
    auto containsSubShape = [&](TopAbs_ShapeEnum shapeType) {
        return BRepUtils::anySubShape(shape, shapeType, [](const TopoDS_Shape&) { return true; });
    };
    if (containsSubShape(TopAbs_EDGE) || containsSubShape(TopAbs_FACE))
        BRepUtils::computeMesh(shape, params, progress);
}

void BRepMeshingUtils::compute(
        const TDF_Label& labelEntity, const BRepMeshingOptions& options, TaskProgress* progress
    )
{
    if (XCaf::isShape(labelEntity))
        BRepMeshingUtils::compute(XCaf::shape(labelEntity), options, progress);
}

} // namespace Mayo
