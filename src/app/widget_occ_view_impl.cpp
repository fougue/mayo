/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/occ_handle.h"
#include "../graphics/graphics_utils.h"

#include <OpenGl_GraphicDriver.hxx>

namespace Mayo {

#if OCC_VERSION_HEX >= 0x070600
OccHandle<Graphic3d_GraphicDriver> QOpenGLWidgetOccView_createCompatibleGraphicsDriver()
{
    auto gfxDriver = new OpenGl_GraphicDriver(GraphicsUtils::AspectDisplayConnection_create(), false/*dontInit*/);
    // Let QOpenGLWidget manage buffer swap
    gfxDriver->ChangeOptions().buffersNoSwap = true;
    // Don't write into alpha channel
    gfxDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // Offscreen FBOs should be always used
    gfxDriver->ChangeOptions().useSystemBuffer = false;

    return gfxDriver;
}
#endif

OccHandle<Graphic3d_GraphicDriver> QWidgetOccView_createCompatibleGraphicsDriver()
{
    return new OpenGl_GraphicDriver(GraphicsUtils::AspectDisplayConnection_create());
}

} // namespace Mayo
