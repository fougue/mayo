/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

// Only meaningful when OCCT is built without Xlib (OpenGl_GraphicDriver then uses EGL)
// Inclusion of that header file should be guarded (UNIX and !APPLE and !XLIB)

#include <OpenGl_GraphicDriver.hxx>

namespace Mayo {

//
// OpenGl_GraphicDriver able to render offscreen without any X11/Wayland connection
//
// OCCT calls eglGetDisplay(EGL_DEFAULT_DISPLAY) in OpenGl_GraphicDriver::InitContext() and then
// picks a config without EGL_SURFACE_TYPE (so EGL_WINDOW_BIT by default). Both are unsuitable for
// headless rendering, hence the EGL display/config/context are created here (surfaceless platform,
// pbuffer-capable config) and given to OpenGl_GraphicDriver::InitEglContext()
//
// Ownership of the EGL display and context is transferred to the base class by setting the
// protected member myIsOwnContext : ~OpenGl_GraphicDriver() -> ReleaseContext() destroys them
//
// Throws Aspect_GraphicDeviceDefinitionError (with an explicit message) when construction fails
//
class EglOffscreenGraphicDriver : public OpenGl_GraphicDriver {
public:
    EglOffscreenGraphicDriver();

    DEFINE_STANDARD_RTTI_INLINE(EglOffscreenGraphicDriver, OpenGl_GraphicDriver)
};

DEFINE_STANDARD_HANDLE(EglOffscreenGraphicDriver, OpenGl_GraphicDriver)

} // namespace Mayo
