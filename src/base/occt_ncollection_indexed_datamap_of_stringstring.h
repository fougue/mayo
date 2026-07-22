/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <NCollection_IndexedDataMap.hxx>
#include <Standard_Version.hxx>
#include <TCollection_AsciiString.hxx>

// TColStd_IndexedDataMapOfStringString is deprecated since OpenCascade v8.0.0
// NCollection_IndexedDataMapOfStringString provides a portable type alias replacment
#if OCC_VERSION_HEX >= 0x070800
using NCollection_IndexedDataMapOfStringString = NCollection_IndexedDataMap<TCollection_AsciiString, TCollection_AsciiString>;
#else
using NCollection_IndexedDataMapOfStringString = NCollection_IndexedDataMap<TCollection_AsciiString,TCollection_AsciiString,TCollection_AsciiString>;
#endif
