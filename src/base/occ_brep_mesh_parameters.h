/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "tkernel_utils.h"
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
#  include <IMeshTools_Parameters.hxx>
#else
#  include <BRepMesh_FastDiscret.hxx>
#endif

namespace Mayo {

class TaskProgress;

// Portable alias over parameters for OpenCascade's built-in BRep mesher
#if OCC_VERSION_HEX >= OCC_VERSION_CHECK(7, 4, 0)
using OccBRepMeshParameters = IMeshTools_Parameters;
#else
using OccBRepMeshParameters = BRepMesh_FastDiscret::Parameters;
#endif

} // namespace Mayo
