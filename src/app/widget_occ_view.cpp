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

#if OCC_VERSION_HEX >= 0x070600
#  include <Aspect_NeutralWindow.hxx>
#  include <OpenGl_Context.hxx>
#  include <OpenGl_GraphicDriver.hxx>
#  include <OpenGl_FrameBuffer.hxx>
#else
#  include "occt_window.h"
#  include <QtGui/QResizeEvent>
#endif

namespace Mayo {

constexpr bool isCoreProfile = false;

WidgetOccView::WidgetOccView(const Handle_V3d_View& view, QWidget* parent)
    : MayoWidgetOccView_ParentType(parent),
      m_view(view)
{
    this->setMouseTracking(true);
    this->setBackgroundRole(QPalette::NoRole);

#if OCC_VERSION_HEX >= 0x070600
    this->setUpdatesEnabled(true);
    this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    QSurfaceFormat glFormat;
    glFormat.setDepthBufferSize(24);
    glFormat.setStencilBufferSize(8);
    glFormat.setVersion(4, 5);
    glFormat.setProfile(isCoreProfile ? QSurfaceFormat::CoreProfile : QSurfaceFormat::CompatibilityProfile);
  #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    glFormat.setColorSpace(QSurfaceFormat::sRGBColorSpace);
    this->setTextureFormat(GL_SRGB8_ALPHA8);
  #endif
    this->setFormat(glFormat);
#else
    // Avoid Qt background clears to improve resizing speed
    this->setAutoFillBackground(false);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_PaintOnScreen);
#endif
}

void WidgetOccView::redraw()
{
#if OCC_VERSION_HEX >= 0x070600
    this->update();
#else
    m_view->Redraw();
#endif
}

#if OCC_VERSION_HEX >= 0x070600
void WidgetOccView::initializeGL()
{
    const QRect wrect = this->rect();
    const Graphic3d_Vec2i viewSize(wrect.right() - wrect.left(), wrect.bottom() - wrect.top());

    Handle_OpenGl_Context glCtx = new OpenGl_Context();
    if (!glCtx->Init(isCoreProfile)) {
        Message::SendFail() << "Error: OpenGl_Context is unable to wrap OpenGL context";
        return;
    }

    auto window = Handle_Aspect_NeutralWindow::DownCast(m_view->Window());
    if (window) {
        window->SetSize(viewSize.x(), viewSize.y());
        m_view->SetWindow(window, glCtx->RenderingContext());
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
        m_view->SetWindow(window, glCtx->RenderingContext());
    }

    //dumpGlInfo(true);
}

void WidgetOccView::paintGL()
{
    if (m_view->Window().IsNull())
        return;

    // Wrap FBO created by QOpenGLWidget
    auto driver = Handle_OpenGl_GraphicDriver::DownCast(m_view->Viewer()->Driver());
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
    auto window = Handle_Aspect_NeutralWindow::DownCast(m_view->Window());
    window->Size(viewSizeOld.x(), viewSizeOld.y());
    if (viewSizeNew != viewSizeOld) {
        window->SetSize(viewSizeNew.x(), viewSizeNew.y());
        m_view->MustBeResized();
        m_view->Invalidate();
    }

    // Redraw the viewer
    //m_view->InvalidateImmediate();
    m_view->Redraw();
}
#else
void WidgetOccView::showEvent(QShowEvent*)
{
    if (m_view->Window().IsNull()) {
        Handle_Aspect_Window hWnd = new OcctWindow(this);
        m_view->SetWindow(hWnd);
        if (!hWnd->IsMapped())
            hWnd->Map();

        m_view->MustBeResized();
    }
}

void WidgetOccView::paintEvent(QPaintEvent*)
{
    m_view->Redraw();
}

void WidgetOccView::resizeEvent(QResizeEvent* event)
{
    if (!event->spontaneous()) // Workaround for infinite window shrink on Ubuntu
        m_view->MustBeResized();
}
#endif

} // namespace Mayo
