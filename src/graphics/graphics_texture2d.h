/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
