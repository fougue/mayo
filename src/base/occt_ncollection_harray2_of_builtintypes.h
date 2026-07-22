/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>

// TColStd_HArray2Of... are deprecated since OpenCascade v8.0.0
// NCollection_HArray2Of... provide portable type alias replacments
#if OCC_VERSION_HEX >= 0x080000
#  include <NCollection_HArray2.hxx>
using NCollection_HArray2OfInteger = NCollection_HArray2<int>;
using NCollection_HArray2OfReal = NCollection_HArray2<double>;
#else
#  include <TColStd_HArray2OfInteger.hxx>
#  include <TColStd_HArray2OfReal.hxx>
using NCollection_HArray2OfInteger = TColStd_HArray2OfInteger;
using NCollection_HArray2OfReal = TColStd_HArray2OfReal;
#endif
