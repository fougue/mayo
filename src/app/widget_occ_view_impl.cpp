/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "../base/occ_handle.h"
#include "../graphics/graphics_utils.h"
#include "../graphics/opengl_utils.h"

#include <OpenGl_GraphicDriver.hxx>

namespace Mayo {

#if OCC_VERSION_HEX >= 0x070600
OccHandle<Graphic3d_GraphicDriver> QOpenGLWidgetOccView_createCompatibleGraphicsDriver()
{
    auto gfxDriver = makeOccHandle<OpenGl_GraphicDriver>(
        GraphicsUtils::AspectDisplayConnection_create(), false/*dontInit*/
    );
    // Let QOpenGLWidget manage buffer swap
    gfxDriver->ChangeOptions().buffersNoSwap = true;
    // Don't write into alpha channel
    gfxDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // Offscreen FBOs should be always used
    gfxDriver->ChangeOptions().useSystemBuffer = false;
    // Disable acclerated context if needed
    const static bool isGpuAccel = OpenGlUtils::isHardwareAccelerationAvailable();
    gfxDriver->ChangeOptions().contextNoAccel = !isGpuAccel;

    return gfxDriver;
}
#endif

} // namespace Mayo
