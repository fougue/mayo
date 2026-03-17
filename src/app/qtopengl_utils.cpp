/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#ifdef _WIN32
#  include <windows.h>
#endif

#include "qtopengl_utils.h"

#include <QtCore/QCoreApplication>

#include <Aspect_NeutralWindow.hxx>
#include <OpenGl_Caps.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_FrameBuffer.hxx>
#include <OpenGl_GlCore20.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <OpenGl_View.hxx>
#include <OpenGl_Window.hxx>
#include <Standard_Version.hxx>

#include <cstring>

namespace Mayo::QtOpenGlUtils {

namespace {

//! Class making DevicePixelRatio() configurable
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


#if OCC_VERSION_HEX >= 0x070600
// OpenGL FBO subclass for wrapping FBO created by Qt using GL_RGBA8 texture format instead of GL_SRGB8_ALPHA8.
// This FBO is set to OpenGl_Context::SetDefaultFrameBuffer() as a final target.
// Subclass calls OpenGl_Context::SetFrameBufferSRGB() with sRGB=false flag,
// which asks OCCT to disable GL_FRAMEBUFFER_SRGB and apply sRGB gamma correction manually.
//
// Note this is using patch https://github.com/gkv311/occt-samples-qopenglwidget/commit/32c997ce281422ce7dcf4f7e1e529fbdf7dc642c
// See also https://github.com/gkv311/occt-samples-qopenglwidget/issues/3
class OcctQtFrameBuffer : public OpenGl_FrameBuffer {
    DEFINE_STANDARD_RTTI_INLINE(OcctQtFrameBuffer, OpenGl_FrameBuffer)
public:
    OcctQtFrameBuffer() = default;

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
#endif

// Helper function to check if application arguments contain any option listed in 'listOption'
// IMPORTANT: capture by reference, because QApplication constructor may alter argc(due to
//            parsing of arguments)
bool argsContainAnyOf(int argc, char* argv[], std::initializer_list<const char*> listOption)
{
    for (int i = 1; i < argc; ++i) {
        for (const char* option : listOption) {
            if (std::strcmp(argv[i], option) == 0)
                return true;
        }
    }

    return false;
}

} // namespace

void platformSetup([[maybe_unused]]int argc, [[maybe_unused]]char* argv[])
{
#if defined(Q_OS_WIN)
    // Never use ANGLE on Windows, since OCCT 3D Viewer does not expect this.
    // Note: use Qt::AA_UseOpenGLES for embedded systems
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    // OpenCascade TKOpenGl depends on XLib for Linux(excepting Android) and BSD systems(excepting macOS)
    // See for example implementation of Aspect_DisplayConnection where XLib is explicitly used
    // On systems running eg Wayland this would cause problems(see https://github.com/fougue/mayo/issues/178)
    // As a workaround the Qt platform is forced to xcb
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || (defined(Q_OS_BSD4) && !defined(Q_OS_MACOS))
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && !argsContainAnyOf(argc, argv, { "-platform" }))
        qputenv("QT_QPA_PLATFORM", "xcb");
#elif defined(Q_OS_HAIKU)
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && !argsContainAnyOf(argc, argv, { "-platform" }))
        qputenv("QT_QPA_PLATFORM", "haiku");
#endif

    // Request Qt to share resources between OpenGL contexts like QOpenGLWidget and QQuickWidget
    // Allows detaching OCCT 3D Viewer widget inside QDockWidget into new top-level window
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

/*
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Workaround for some bugs in Qt5
    if (!qEnvironmentVariableIsSet("QT_ENABLE_GLYPH_CACHE_WORKAROUND"))
        qputenv("QT_ENABLE_GLYPH_CACHE_WORKAROUND", "1");
#endif
*/

    // Global OpenGL setup managed by Qt
    QSurfaceFormat::setDefaultFormat(
        QtOpenGlUtils::surfaceFormat(QSurfaceFormat::NoProfile, false/*!debug*/)
    );

    // Ask Qt managing rendering from GUI thread instead of QSGRenderThread for QtQuick applications
    /*
    if (!qEnvironmentVariableIsSet("QSG_RENDER_LOOP"))
        qputenv("QSG_RENDER_LOOP", "basic");
    */

// Enable auto-scaling for high-density screens and fractional scale factors(this is default since Qt6)
/*
#if (QT_VERSION_MAJOR == 5)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  #if (QT_VERSION_MINOR >= 14)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
  #endif
#endif
*/
}

QSurfaceFormat surfaceFormat(QSurfaceFormat::OpenGLContextProfile profile, bool debug)
{
    const bool isDeepColor = false;
    if (profile == QSurfaceFormat::NoProfile) {
        profile = QSurfaceFormat::CompatibilityProfile;
#ifdef __APPLE__
        // Suppress Qt warning "QCocoaGLContext: Falling back to unshared context"
        profile = QSurfaceFormat::CoreProfile;
#endif
    }

    QSurfaceFormat glFormat;
    if (isDeepColor) {
        glFormat.setRedBufferSize(10);
        glFormat.setGreenBufferSize(10);
        glFormat.setBlueBufferSize(10);
        glFormat.setAlphaBufferSize(2);
    }

    glFormat.setDepthBufferSize(24);
    glFormat.setStencilBufferSize(8);
    glFormat.setProfile(profile);
    if (profile == QSurfaceFormat::CoreProfile)
        glFormat.setVersion(4, 5);

    // Request sRGBColorSpace color-space to meet OCCT expectations or use OcctQtFrameBuffer fallback
    /*
#if (QT_VERSION_MAJOR > 5) || (QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR >= 10)
    glFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
#endif
    */

    if (debug)
        glFormat.setOption(QSurfaceFormat::DebugContext, true);

    return glFormat;
}

void setCapsFromSurfaceFormat(OpenGl_Caps& caps, const QSurfaceFormat& format)
{
    caps.contextDebug = format.testOption(QSurfaceFormat::DebugContext);
    caps.contextSyncDebug = caps.contextDebug;
    caps.contextCompatible = format.profile() != QSurfaceFormat::CoreProfile;
#if (OCC_VERSION_HEX >= 0x070700)
    caps.buffersDeepColor =
        format.redBufferSize() == 10
        && format.greenBufferSize() == 10
        && format.blueBufferSize() == 10
    ;
#endif
}


#if OCC_VERSION_HEX >= 0x070600

Aspect_Drawable glNativeWindow(Aspect_Drawable nativeWin)
{
#ifdef Q_OS_WIN
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

void resetGlStateBeforeOcct(const OccHandle<V3d_View>& view)
{
    OccHandle<OpenGl_Context> glCtx = QtOpenGlUtils::glContext(view);
    if (glCtx.IsNull())
        return;

    if (glCtx->core20fwd != nullptr) {
        // Shouldn't be a problem in most cases, but make sure to unbind active GLSL program
        glCtx->core20fwd->glUseProgram(0);
    }

    // Qt leaves GL_BLEND enabled after drawing semitransparent elements, but OCCT doesn't reset its
    // state before drawing opaque objects. Disable also texture bindings left by Qt.
    glCtx->core11fwd->glBindTexture(GL_TEXTURE_2D, 0);
    glCtx->core11fwd->glDisable(GL_BLEND);
    if (glCtx->core11ffp != nullptr) {
        glCtx->core11fwd->glDisable(GL_ALPHA_TEST);
        glCtx->core11fwd->glDisable(GL_TEXTURE_2D);
    }
}

void resetGlStateAfterOcct(const OccHandle<V3d_View>& view)
{
    OccHandle<OpenGl_Context> glCtx = QtOpenGlUtils::glContext(view);
    if (glCtx.IsNull())
        return;

    // Qt expects default OpenGL pack/unpack alignment setup, while OCCT manages it dynamically
    // Without resetting alignment setup, Qt will draw some textures corrupted(like fonts)
    glCtx->core11fwd->glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glCtx->core11fwd->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (glCtx->core15fwd != nullptr) {
        // Qt expects first texture object to be bound,
        // but OCCT might leave another object bound within multi-texture mapping
        glCtx->core15fwd->glActiveTexture(GL_TEXTURE0);
    }
}

bool initializeGlWindow(
        const OccHandle<V3d_View>& view, Aspect_Drawable nativeWin, const Graphic3d_Vec2i& size, double pixelRatio
    )
{
    auto driver = OccHandle<OpenGl_GraphicDriver>::DownCast(view->Viewer()->Driver());
    OccHandle<OpenGl_Context> glCtx = new OpenGl_Context;
    if (!glCtx->Init(!driver->Options().contextCompatible)) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return false;
    }

    auto window = OccHandle<OcctNeutralWindow>::DownCast(view->Window());
    if (window.IsNull()) {
        window = new OcctNeutralWindow;
        window->SetVirtual(true);
    }

    window->SetNativeHandle(QtOpenGlUtils::glNativeWindow(nativeWin));
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

bool initializeGlFramebufferObject(const OccHandle<V3d_View>& view)
{
    OccHandle<OpenGl_Context> glCtx = QtOpenGlUtils::glContext(view);
    OccHandle<OpenGl_FrameBuffer> defaultFbo = glCtx->DefaultFrameBuffer();
    if (defaultFbo.IsNull()) {
        defaultFbo = new OcctQtFrameBuffer;
        glCtx->SetDefaultFrameBuffer(defaultFbo);
    }

    if (!defaultFbo->InitWrapper(glCtx)) {
        defaultFbo.Nullify();
        Message::DefaultMessenger()->Send("Default FBO wrapper creation failed", Message_Fail);
        return false;
    }

    Graphic3d_Vec2i viewSizeOld;
    const Graphic3d_Vec2i viewSizeNew = defaultFbo->GetVPSize();
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

} // namespace Mayo::QtOpenGlUtils
