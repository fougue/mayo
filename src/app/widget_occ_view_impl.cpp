/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/occ_handle.h"
#include "../graphics/graphics_utils.h"

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX >= 0x070600
#  include <Aspect_NeutralWindow.hxx>
#  include <OpenGl_Context.hxx>
#  include <OpenGl_FrameBuffer.hxx>
#endif

#include <functional>

namespace Mayo {

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

    void BindBuffer(const OccHandle<OpenGl_Context>& ctx) override
    {
        OpenGl_FrameBuffer::BindBuffer(ctx);
        ctx->SetFrameBufferSRGB(true, false);
        // NOTE: commenting the line just above makes the FBO to work on some configs(eg VM Ubuntu 18.04)
    }

    void BindDrawBuffer(const OccHandle<OpenGl_Context>& ctx) override
    {
        OpenGl_FrameBuffer::BindDrawBuffer(ctx);
        ctx->SetFrameBufferSRGB(true, false);
    }

    void BindReadBuffer(const OccHandle<OpenGl_Context>& ctx) override
    {
        OpenGl_FrameBuffer::BindReadBuffer(ctx);
    }
};

} // namespace

bool QOpenGLWidgetOccView_isCoreProfile()
{
    return false;
}

void QOpenGLWidgetOccView_createOpenGlContext(std::function<void(Aspect_RenderingContext)> fnCallback)
{
    auto glCtx = makeOccHandle<OpenGl_Context>();
    if (!glCtx->Init(QOpenGLWidgetOccView_isCoreProfile())) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return;
    }

    if (fnCallback)
        fnCallback(glCtx->RenderingContext());
}

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

bool QOpenGLWidgetOccView_wrapFrameBuffer(const OccHandle<Graphic3d_GraphicDriver>& gfxDriver)
{
    // Wrap FBO created by QOpenGLWidget
    auto driver = OccHandle<OpenGl_GraphicDriver>::DownCast(gfxDriver);
    if (!driver)
        return false;

    const OccHandle<OpenGl_Context>& glCtx = driver->GetSharedContext();
    OccHandle<OpenGl_FrameBuffer> defaultFbo = glCtx->DefaultFrameBuffer();
    if (!defaultFbo) {
        //defaultFbo = new OpenGl_FrameBuffer;
        defaultFbo = new QtOccFrameBuffer;
        glCtx->SetDefaultFrameBuffer(defaultFbo);
    }

    if (!defaultFbo->InitWrapper(glCtx)) {
        defaultFbo.Nullify();
        Message::SendFail() << "Default FBO wrapper creation failed";
        return false;
    }

    return true;
}

Graphic3d_Vec2i QOpenGLWidgetOccView_getDefaultframeBufferViewportSize(const OccHandle<Graphic3d_GraphicDriver>& gfxDriver)
{
    auto driver = OccHandle<OpenGl_GraphicDriver>::DownCast(gfxDriver);
    return driver->GetSharedContext()->DefaultFrameBuffer()->GetVPSize();
}

#endif // OCC_VERSION_HEX >= 0x070600

OccHandle<Graphic3d_GraphicDriver> QWidgetOccView_createCompatibleGraphicsDriver()
{
    return new OpenGl_GraphicDriver(GraphicsUtils::AspectDisplayConnection_create());
}

} // namespace Mayo
