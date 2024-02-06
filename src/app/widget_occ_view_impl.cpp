/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QtGlobal>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070600
#  include <Aspect_NeutralWindow.hxx>
#  include <OpenGl_Context.hxx>
#  include <OpenGl_FrameBuffer.hxx>
#  include <OpenGl_View.hxx>
#  include <OpenGl_Window.hxx>
#endif

#include <functional>

namespace Mayo {

namespace {

Handle_Aspect_DisplayConnection createDisplayConnection()
{
#if (!defined(Q_OS_WIN) && (!defined(Q_OS_MAC) || defined(MACOSX_USE_GLX)))
    return new Aspect_DisplayConnection(std::getenv("DISPLAY"));
#else
    return new Aspect_DisplayConnection;
#endif
}

} // namespace

#if OCC_VERSION_HEX >= 0x070600
namespace {

// OpenGL FBO subclass for wrapping FBO created by Qt using GL_RGBA8 texture format instead of GL_SRGB8_ALPHA8.
// This FBO is set to OpenGl_Context::SetDefaultFrameBuffer() as a final target.
// Subclass calls OpenGl_Context::SetFrameBufferSRGB() with sRGB=false flag,
// which asks OCCT to disable GL_FRAMEBUFFER_SRGB and apply sRGB gamma correction manually.
//
// Note this is using patch https://github.com/gkv311/occt-samples-qopenglwidget/commit/32c997ce281422ce7dcf4f7e1e529fbdf7dc642c
// See also https://github.com/gkv311/occt-samples-qopenglwidget/issues/3
class QtOccFrameBuffer : public OpenGl_FrameBuffer {
    DEFINE_STANDARD_RTTI_INLINE(QtOccFrameBuffer, OpenGl_FrameBuffer)
public:
    QtOccFrameBuffer() {}

    void BindBuffer(const Handle(OpenGl_Context)& ctx) override
    {
        OpenGl_FrameBuffer::BindBuffer(ctx);
        ctx->SetFrameBufferSRGB(true, false);
        // NOTE: commenting the line just above makes the FBO to work on some configs(eg VM Ubuntu 18.04)
    }

    void BindDrawBuffer(const Handle(OpenGl_Context)& ctx) override
    {
        OpenGl_FrameBuffer::BindDrawBuffer(ctx);
        ctx->SetFrameBufferSRGB(true, false);
    }

    void BindReadBuffer(const Handle(OpenGl_Context)& ctx) override
    {
        OpenGl_FrameBuffer::BindReadBuffer(ctx);
    }
};

Handle_OpenGl_Context QOpenGLWidgetOccView_getOpenGlContext(const Handle_Graphic3d_CView& view)
{
    Handle_OpenGl_View glView = Handle_OpenGl_View::DownCast(view);
    return glView->GlWindow()->GetGlContext();
}

} // namespace

bool QOpenGLWidgetOccView_isCoreProfile()
{
    return true;
}

void QOpenGLWidgetOccView_createOpenGlContext(std::function<void(Aspect_RenderingContext)> fnCallback)
{
    Handle_OpenGl_Context glCtx = new OpenGl_Context;
    if (!glCtx->Init(QOpenGLWidgetOccView_isCoreProfile())) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return;
    }

    if (fnCallback)
        fnCallback(glCtx->RenderingContext());
}

Handle_Graphic3d_GraphicDriver QOpenGLWidgetOccView_createCompatibleGraphicsDriver()
{
    auto gfxDriver = new OpenGl_GraphicDriver(createDisplayConnection(), false/*dontInit*/);
    // Let QOpenGLWidget manage buffer swap
    gfxDriver->ChangeOptions().buffersNoSwap = true;
    // Don't write into alpha channel
    gfxDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // Offscreen FBOs should be always used
    gfxDriver->ChangeOptions().useSystemBuffer = false;
    return gfxDriver;
}

bool QOpenGLWidgetOccView_wrapFrameBuffer(const Handle_Graphic3d_CView& view)
{
    // Wrap FBO created by QOpenGLWidget
    Handle_OpenGl_Context glContext = QOpenGLWidgetOccView_getOpenGlContext(view);
    Handle_OpenGl_FrameBuffer defaultFbo = glContext->DefaultFrameBuffer();
    if (!defaultFbo) {
        defaultFbo = new QtOccFrameBuffer;
        glContext->SetDefaultFrameBuffer(defaultFbo);
    }

    if (!defaultFbo->InitWrapper(glContext)) {
        defaultFbo.Nullify();
        Message::SendFail() << "Default FBO wrapper creation failed";
        return false;
    }

    return true;
}

Graphic3d_Vec2i QOpenGLWidgetOccView_getDefaultframeBufferViewportSize(const Handle_Graphic3d_CView& view)
{
    Handle_OpenGl_Context glContext = QOpenGLWidgetOccView_getOpenGlContext(view);
    return glContext->DefaultFrameBuffer()->GetVPSize();
}

#endif // OCC_VERSION_HEX >= 0x070600

Handle_Graphic3d_GraphicDriver QWidgetOccView_createCompatibleGraphicsDriver()
{
    return new OpenGl_GraphicDriver(createDisplayConnection());
}

} // namespace Mayo
