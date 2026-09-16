/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#ifdef _WIN32
#  include <windows.h>
#endif

#include "../base/global.h"
#include "opengl_utils.h"

#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Window.hxx>

#if defined(__APPLE__)
#  include <OpenGL/OpenGL.h>
#endif

namespace Mayo::OpenGlUtils {

namespace {

// Class making DevicePixelRatio() configurable
class OcctNeutralWindow : public Aspect_NeutralWindow {
public:
    OcctNeutralWindow() = default;
#if OCC_VERSION_HEX >= 0x070600
    double DevicePixelRatio() const override { return m_pixelRatio; }
#endif
    void SetDevicePixelRatio(double ratio) { m_pixelRatio = ratio; }

private:
    double m_pixelRatio = 1.;
};

} // namespace

Aspect_Drawable glNativeWindow(Aspect_Drawable nativeWin)
{
#ifdef MAYO_OS_WINDOWS
    HDC wglDevCtx = wglGetCurrentDC();
    HWND wglWin = WindowFromDC(wglDevCtx);
    nativeWin = (Aspect_Drawable)wglWin;
#endif

    return nativeWin;
}

OccHandle<OpenGl_Context> glContext(const OccHandle<V3d_View>& view)
{
    auto glView = OccHandle<OpenGl_View>::DownCast(view->View());
    return glView->GlWindow()->GetGlContext();
}

bool isHardwareAccelerationAvailable()
{
#if defined(__APPLE__)
    const CGLPixelFormatAttribute attribs[] = {
        kCGLPFAAccelerated,
        (CGLPixelFormatAttribute)0
    };

    CGLPixelFormatObj pixelFormat = nullptr;
    GLint numVirtualScreens = 0;
    const CGLError err = CGLChoosePixelFormat(attribs, &pixelFormat, &numVirtualScreens);
    const bool ok = (err == kCGLNoError) && (pixelFormat != nullptr);

    if (pixelFormat != nullptr)
        CGLDestroyPixelFormat(pixelFormat);

    return ok;
#else
    return true;
#endif
}

#if OCC_VERSION_HEX >= 0x070600

bool initializeGlWindow(
        const OccHandle<V3d_View>& view, Aspect_Drawable nativeWin, const NCollection_Vec2<int>& size, double pixelRatio
    )
{
    auto driver = OccHandle<OpenGl_GraphicDriver>::DownCast(view->Viewer()->Driver());
    auto glCtx = makeOccHandle<OpenGl_Context>();
    if (!glCtx->Init(!driver->Options().contextCompatible)) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return false;
    }

    auto window = OccHandle<OcctNeutralWindow>::DownCast(view->Window());
    if (window.IsNull()) {
        window = makeOccHandle<OcctNeutralWindow>();
        window->SetVirtual(true);
    }

    window->SetNativeHandle(OpenGlUtils::glNativeWindow(nativeWin));
    window->SetSize(size.x(), size.y());
    window->SetDevicePixelRatio(pixelRatio);
    view->SetWindow(window, glCtx->RenderingContext());
    view->MustBeResized();
    view->Invalidate();
#if OCC_VERSION_HEX >= 0x070700
    for (const OccHandle<V3d_View>& subview : view->Subviews()) {
        subview->MustBeResized();
        subview->Invalidate();
    }
#endif

    return true;
}

bool initializeGlFramebufferObject(
        const OccHandle<V3d_View>& view,
        const std::function<OccHandle<OpenGl_FrameBuffer>()>& createFbo
    )
{
    OccHandle<OpenGl_Context> glCtx = OpenGlUtils::glContext(view);
    OccHandle<OpenGl_FrameBuffer> defaultFbo = glCtx->DefaultFrameBuffer();
    if (defaultFbo.IsNull() && createFbo) {
        defaultFbo = createFbo();
        glCtx->SetDefaultFrameBuffer(defaultFbo);
    }

    if (defaultFbo.IsNull() || !defaultFbo->InitWrapper(glCtx)) {
        defaultFbo.Nullify();
        Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
        return false;
    }

    NCollection_Vec2<int> viewSizeOld;
    const NCollection_Vec2<int> viewSizeNew = defaultFbo->GetVPSize();
    auto window = OccHandle<OcctNeutralWindow>::DownCast(view->Window());
    window->Size(viewSizeOld.x(), viewSizeOld.y());
    if (viewSizeNew != viewSizeOld) {
        window->SetSize(viewSizeNew.x(), viewSizeNew.y());
        view->MustBeResized();
        view->Invalidate();
#if OCC_VERSION_HEX >= 0x070700
        for (const OccHandle<V3d_View>& subview : view->Subviews()) {
            subview->MustBeResized();
            subview->Invalidate();
            defaultFbo->SetupViewport(glCtx);
        }
#endif
    }

    return true;
}
#endif // OCC_VERSION_HEX >= 0x070600

} // namespace Mayo::OpenGlUtils
