/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>

// TColgp_HArray1OfPnt is deprecated since OpenCascade v8.0.0
// NCollection_HArray1OfPnt provides a portable type alias replacment
#if OCC_VERSION_HEX >= 0x080000
#  include <NCollection_HArray1.hxx>
using NCollection_HArray1OfPnt = NCollection_HArray1<gp_Pnt>;
#else
#  include <TColgp_HArray1OfPnt.hxx>
using NCollection_HArray1OfPnt = TColgp_HArray1OfPnt;
#endif
