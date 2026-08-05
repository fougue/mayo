/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>
#include <IMeshTools_Parameters.hxx>

namespace Mayo {

// Portable alias over parameters for OpenCascade's built-in BRep mesher
using OccBRepMeshParameters = IMeshTools_Parameters;

} // namespace Mayo
