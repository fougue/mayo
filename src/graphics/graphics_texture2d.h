/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070700
#  include <Graphic3d_Texture2D.hxx>
#else
#  include <Graphic3d_Texture2Dmanual.hxx>
#endif

namespace Mayo {

#if OCC_VERSION_HEX >= 0x070700
using GraphicsTexture2D = Graphic3d_Texture2D;
#else
using GraphicsTexture2D = Graphic3d_Texture2Dmanual;
#endif

} // namespace Mayo
