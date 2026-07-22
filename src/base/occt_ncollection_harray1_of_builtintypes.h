/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>

// TColStd_HArray1Of... are deprecated since OpenCascade v8.0.0
// NCollection_HArray1Of... provide portable type alias replacments
#if OCC_VERSION_HEX >= 0x080000
#  include <NCollection_HArray1.hxx>
using NCollection_HArray1OfInteger = NCollection_HArray1<int>;
using NCollection_HArray1OfReal = NCollection_HArray1<double>;
#else
#  include <TColStd_HArray1OfInteger.hxx>
#  include <TColStd_HArray1OfReal.hxx>
using NCollection_HArray1OfInteger = TColStd_HArray1OfInteger;
using NCollection_HArray1OfReal = TColStd_HArray1OfReal;
#endif
