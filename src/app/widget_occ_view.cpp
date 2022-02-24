/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QtGlobal>
#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#include "widget_occ_view.h"

#include "occt_window.h"

#include <QtGui/QResizeEvent>

#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#if OCC_VERSION_HEX >= 0x070600
#  include <Aspect_NeutralWindow.hxx>
#  include <OpenGl_Context.hxx>
#  include <OpenGl_FrameBuffer.hxx>
#endif

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

IWidgetOccView::Creator& getWidgetOccViewCreator()
{
    static IWidgetOccView::Creator fn = [](const Handle_V3d_View& view, QWidget* parent) {
        return new QWidgetOccView(view, parent);
    };
    return fn;
}

} // namespace


void IWidgetOccView::setCreator(IWidgetOccView::Creator fn)
{
    getWidgetOccViewCreator() = std::move(fn);
}

IWidgetOccView* IWidgetOccView::create(const Handle_V3d_View& view, QWidget* parent)
{
    auto& fn = getWidgetOccViewCreator();
    return fn(view, parent);
}


#if OCC_VERSION_HEX >= 0x070600

constexpr bool isCoreProfile = true;

QOpenGLWidgetOccView::QOpenGLWidgetOccView(const Handle_V3d_View& view, QWidget* parent)
    : QOpenGLWidget(parent),
      IWidgetOccView(view)
{
    this->setMouseTracking(true);
    this->setBackgroundRole(QPalette::NoRole);

    this->setUpdatesEnabled(true);
    this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    QSurfaceFormat glFormat;
    glFormat.setDepthBufferSize(24);
    glFormat.setStencilBufferSize(8);
    if (isCoreProfile)
        glFormat.setVersion(4, 5);

    glFormat.setProfile(isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    glFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
    this->setTextureFormat(GL_SRGB8_ALPHA8);
#endif
    this->setFormat(glFormat);
}

void QOpenGLWidgetOccView::redraw()
{
    this->update();
}

QOpenGLWidgetOccView* QOpenGLWidgetOccView::create(const Handle_V3d_View& view, QWidget* parent)
{
    return new QOpenGLWidgetOccView(view, parent);
}

Handle_Graphic3d_GraphicDriver QOpenGLWidgetOccView::createCompatibleGraphicsDriver()
{
    auto gfxDriver = new OpenGl_GraphicDriver(createDisplayConnection(), false/*dontInit*/);
    // Let QOpenGLWidget manage buffer swap
    gfxDriver->ChangeOptions().buffersNoSwap = true;
    // Don't write into alpha channel
    gfxDriver->ChangeOptions().buffersOpaqueAlpha = true;
    // Offscreen FBOs should be always used
    gfxDriver->ChangeOptions().useSystemBuffer = false;
#  if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    Message::SendWarning("Warning! Qt 5.10+ is required for sRGB setup.\n"
                         "Colors in 3D Viewer might look incorrect (Qt " QT_VERSION_STR " is used).\n");
    gfxDriver->ChangeOptions().sRGBDisable = true;
#  endif

    return gfxDriver;
}

void QOpenGLWidgetOccView::initializeGL()
{
    const QRect wrect = this->rect();
    const Graphic3d_Vec2i viewSize(wrect.right() - wrect.left(), wrect.bottom() - wrect.top());

    Handle_OpenGl_Context glCtx = new OpenGl_Context();
    if (!glCtx->Init(isCoreProfile)) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return;
    }

    auto window = Handle_Aspect_NeutralWindow::DownCast(this->v3dView()->Window());
    if (window) {
        window->SetSize(viewSize.x(), viewSize.y());
        this->v3dView()->SetWindow(window, glCtx->RenderingContext());
    }
    else {
        window = new Aspect_NeutralWindow;
        Aspect_Drawable nativeWin = nullptr;
#ifdef Q_OS_WIN
        HDC  wglDevCtx = wglGetCurrentDC();
        HWND wglWin = WindowFromDC(wglDevCtx);
        nativeWin = (Aspect_Drawable)wglWin;
#else
        nativeWin = (Aspect_Drawable)this->winId();
#endif
        window->SetNativeHandle(nativeWin);
        window->SetSize(viewSize.x(), viewSize.y());
        this->v3dView()->SetWindow(window, glCtx->RenderingContext());
    }

    //dumpGlInfo(true);
}

void QOpenGLWidgetOccView::paintGL()
{
    if (this->v3dView()->Window().IsNull())
        return;

    // Wrap FBO created by QOpenGLWidget
    auto driver = Handle_OpenGl_GraphicDriver::DownCast(this->v3dView()->Viewer()->Driver());
    const Handle_OpenGl_Context& glCtx = driver->GetSharedContext();
    Handle_OpenGl_FrameBuffer defaultFbo = glCtx->DefaultFrameBuffer();
    if (!defaultFbo) {
        defaultFbo = new OpenGl_FrameBuffer();
        glCtx->SetDefaultFrameBuffer(defaultFbo);
    }

    if (!defaultFbo->InitWrapper(glCtx)) {
      defaultFbo.Nullify();
      Message::SendFail() << "Default FBO wrapper creation failed";
      return;
    }

    Graphic3d_Vec2i viewSizeOld;
    const Graphic3d_Vec2i viewSizeNew = defaultFbo->GetVPSize();
    auto window = Handle_Aspect_NeutralWindow::DownCast(this->v3dView()->Window());
    window->Size(viewSizeOld.x(), viewSizeOld.y());
    if (viewSizeNew != viewSizeOld) {
        window->SetSize(viewSizeNew.x(), viewSizeNew.y());
        this->v3dView()->MustBeResized();
        this->v3dView()->Invalidate();
    }

    // Redraw the viewer
    //this->v3dView()->InvalidateImmediate();
    this->v3dView()->Redraw();
}

#endif // OCC_VERSION_HEX >= 0x070600


QWidgetOccView::QWidgetOccView(const Handle_V3d_View& view, QWidget* parent)
    : QWidget(parent),
      IWidgetOccView(view)
{
    this->setMouseTracking(true);
    this->setBackgroundRole(QPalette::NoRole);

    // Avoid Qt background clears to improve resizing speed
    this->setAutoFillBackground(false);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_PaintOnScreen);
}

Handle_Graphic3d_GraphicDriver QWidgetOccView::createCompatibleGraphicsDriver()
{
    return new OpenGl_GraphicDriver(createDisplayConnection());
}

void QWidgetOccView::redraw()
{
    this->v3dView()->Redraw();
}

QWidgetOccView* QWidgetOccView::create(const Handle_V3d_View& view, QWidget* parent)
{
    return new QWidgetOccView(view, parent);
}

void QWidgetOccView::showEvent(QShowEvent*)
{
    if (this->v3dView()->Window().IsNull()) {
        Handle_Aspect_Window hWnd = new OcctWindow(this);
        this->v3dView()->SetWindow(hWnd);
        if (!hWnd->IsMapped())
            hWnd->Map();

        this->v3dView()->MustBeResized();
    }
}

void QWidgetOccView::paintEvent(QPaintEvent*)
{
    this->v3dView()->Redraw();
}

void QWidgetOccView::resizeEvent(QResizeEvent* event)
{
    if (!event->spontaneous()) // Workaround for infinite window shrink on Ubuntu
        this->v3dView()->MustBeResized();
}

} // namespace Mayo
