/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

// --
// NOTE
// This file isolates inclusion of <Aspect_DisplayConnection.hxx> which is problematic on X11/Linux
// <X.h> #defines constants like "None" which causes name clash with GuiDocument::ViewTrihedronMode::None
// --

#include "../base/occ_handle.h"
#include "../graphics/graphics_utils.h"
#include "../graphics/opengl_utils.h"

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <functional>

namespace Mayo {

using FunctionCreateGraphicsDriver = std::function<OccHandle<Graphic3d_GraphicDriver>()>;

OccHandle<Graphic3d_GraphicDriver> createDefaultGraphicsDriver()
{
    auto driver = makeOccHandle<OpenGl_GraphicDriver>(
        GraphicsUtils::AspectDisplayConnection_create(), false/*dontInit*/
    );
    const static bool isGpuAccel = OpenGlUtils::isHardwareAccelerationAvailable();
    driver->ChangeOptions().contextNoAccel = !isGpuAccel;
    if (!driver->InitContext()) {
        // TODO Prompt error message
    }

    return driver;
}

static FunctionCreateGraphicsDriver& getFunctionCreateGraphicsDriver()
{
    static FunctionCreateGraphicsDriver fn = &createDefaultGraphicsDriver;
    return fn;
}

void setFunctionCreateGraphicsDriver(FunctionCreateGraphicsDriver fn)
{
    getFunctionCreateGraphicsDriver() = std::move(fn);
}

OccHandle<Graphic3d_GraphicDriver> graphicsCreateDriver()
{
    const auto& fn = getFunctionCreateGraphicsDriver();
    if (fn)
        return fn();

    return {};
}

} // namespace Mayo
