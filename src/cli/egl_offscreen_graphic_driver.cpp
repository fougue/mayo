/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "egl_offscreen_graphic_driver.h"

#include "../base/global.h"
#include "../base/occ_handle.h"

#include <Aspect_DisplayConnection.hxx>
#include <Aspect_GraphicDeviceDefinitionError.hxx>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cstring>

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#  define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif

namespace Mayo {

namespace {

const char* eglErrorName(EGLint code)
{
    switch (code) {
    case EGL_SUCCESS: return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
    case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
    case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
    case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
    case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
    case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
    case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
    case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
    default: return "unknown";
    }
}

// Builds "EglOffscreenGraphicDriver: <what> (EGL error EGL_BAD_ALLOC, 0x3003)"
// Must be called right after the failing EGL call, before any other EGL call, because eglGetError()
// returns (and resets) the last error of the calling thread
// The suffix is omitted when no EGL error is pending: some failures, like zero matching configs
// from eglChooseConfig(), are not reported as EGL errors
std::string eglFailure(const char* what)
{
    const EGLint code = eglGetError();
    std::string msg = std::string{"EglOffscreenGraphicDriver: "} + what;
    if (code != EGL_SUCCESS) {
        char hex[16];
        std::snprintf(hex, sizeof(hex), "0x%04X", static_cast<unsigned>(code));
        msg += std::string{" (EGL error "} + eglErrorName(code) + ", " + hex + ")";
    }

    return msg;
}

// Returns nullptr on success, otherwise a static error message
std::string createEglObjects(EGLDisplay& display, EGLConfig& config, EGLContext& context)
{
    const char* clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!clientExts || !std::strstr(clientExts, "EGL_MESA_platform_surfaceless"))
        return eglFailure("EGL_MESA_platform_surfaceless is not available");

    auto getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
        eglGetProcAddress("eglGetPlatformDisplayEXT")
    );
    if (!getPlatformDisplay)
        return eglFailure("eglGetPlatformDisplayEXT is not available");

    display = getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
    if (display == EGL_NO_DISPLAY)
        return eglFailure("no EGL surfaceless display");

    if (!eglInitialize(display, nullptr, nullptr))
        return eglFailure("eglInitialize() failed on surfaceless platform");

    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    EGLint configCount = 0;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &configCount) || configCount < 1)
        return eglFailure("no EGL config with OpenGL + pbuffer support");

    if (!eglBindAPI(EGL_OPENGL_API))
        return eglFailure("EGL does not provide the OpenGL API");

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, nullptr);
    if (context == EGL_NO_CONTEXT)
        return eglFailure("eglCreateContext() failed");

    return {};
}

void destroyEglObjects(EGLDisplay display, EGLContext context)
{
    if (display == EGL_NO_DISPLAY)
        return;

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context != EGL_NO_CONTEXT)
        eglDestroyContext(display, context);

    eglTerminate(display);
}

} // namespace

EglOffscreenGraphicDriver::EglOffscreenGraphicDriver()
    : OpenGl_GraphicDriver(makeOccHandle<Aspect_DisplayConnection>(), false/*dontInit*/)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLConfig config = nullptr;
    const std::string error = createEglObjects(display, config, context);
    if (!error.empty()) {
        destroyEglObjects(display, context);
        throw Aspect_GraphicDeviceDefinitionError(error.c_str());
    }

    bool isInitialized = false;
    try {
        isInitialized = this->InitEglContext(display, context, config);
    }
    catch (...) {
        destroyEglObjects(display, context);
        throw;
    }

    if (!isInitialized) {
        destroyEglObjects(display, context);
        throw Aspect_GraphicDeviceDefinitionError(
            "EglOffscreenGraphicDriver: OpenGl_GraphicDriver::InitEglContext() failed"
        );
    }

    // The base class now owns display and context, and releases them in its destructor
    myIsOwnContext = true;
}

} // namespace Mayo
