/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/quantity.h"

class TopoDS_Shape;
class TDF_Label;

namespace Mayo {

class TaskProgress;

// Meshing options for BRep shapes, intended for OpenCascade BRepMesh_IncrementalMesh
struct BRepMeshingOptions {
    // Predefined meshing quality levels
    enum class Quality {
        VeryCoarse, Coarse, Normal, Precise, VeryPrecise, UserDefined
    };

    // Selected meshing quality
    // If set to UserDefined, the custom* members are used instead of the predefined quality
    // coefficients
    Quality quality{Quality::Normal};

    // ⚠ Following members are relevant only when quality ≠ UserDefined

    struct Coefficients {
        double chordalDeflection;
        double angularDeflection;
    };
    // Coefficients associated with each predefined quality level
    struct QualityCoefficients {
        Coefficients veryCoarse;
        Coefficients coarse;
        Coefficients normal;
        Coefficients precise;
        Coefficients veryPrecise;
    };

    // Table of coefficients for predefined quality levels
    // If null, defaultQualityCoefficients() is used
    const QualityCoefficients* qualityCoefficients = nullptr;

    // Returns the coefficients associated with the given quality level
    const Coefficients& coefficients(Quality quality) const;

    // Returns the default coefficients for the predefined quality levels
    static const QualityCoefficients& defaultQualityCoefficients();

    // ⚠ Following members are relevant only when quality = UserDefined

    // Same meaning as IMeshTools_Parameters::Deflection
    QuantityLength customChordalDeflection{1 * Quantity_Millimeter};

    // Same meaning as IMeshTools_Parameters::Angle
    QuantityAngle customAngularDeflection{20 * Quantity_Degree};

    // Same meaning as IMeshTools_Parameters::Relative
    bool customRelative{false};

};

struct BRepMeshingUtils {
    // Returns an estimate of a suitable absolute chordal deflection for the given `shape`
    // The estimate is based on the shape bounding box using the same heuristic as
    // OpenCascade Prs3d::GetDeflection()
    static QuantityLength chordalDeflectionEstimate(const TopoDS_Shape& shape);

    // Computes the mesh (triangulations) of the given shape
    static void compute(
        const TopoDS_Shape& shape, const BRepMeshingOptions& options, TaskProgress* progress = nullptr
    );

    // Computes the mesh (triangulations) of the shape referenced by the given XDE label
    static void compute(
        const TDF_Label& labelEntity, const BRepMeshingOptions& options, TaskProgress* progress = nullptr
    );
};

} // namespace Mayo
