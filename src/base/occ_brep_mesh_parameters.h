/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
