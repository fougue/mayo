/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070400
#  include <IMeshTools_Parameters.hxx>
#else
#  include <BRepMesh_FastDiscret.hxx>
#endif

namespace Mayo {

// Portable alias over parameters for OpenCascade's built-in BRep mesher
#if OCC_VERSION_HEX >= 0x070400
using OccBRepMeshParameters = IMeshTools_Parameters;
#else
using OccBRepMeshParameters = BRepMesh_FastDiscret::Parameters;
#endif

} // namespace Mayo
