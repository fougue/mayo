/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include <QtCore/QtGlobal>
#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#include "../base/occ_handle.h"
#include "../graphics/graphics_utils.h"
#include "occt_window.h"
#include "qtopengl_utils.h"
#include "widget_occ_view.h"

#include <QtGui/QResizeEvent>
#include <OpenGl_GraphicDriver.hxx>

namespace Mayo {

static void enableMsaa4x(const OccHandle<V3d_View>& view)
{
    if (view->Viewer()->Driver()->InquireLimit(Graphic3d_TypeOfLimit_MaxMsaa) >= 4)
        view->ChangeRenderingParams().NbMsaaSamples = 4;
}

static IWidgetOccView::Creator& getWidgetOccViewCreator()
{
    static IWidgetOccView::Creator fn = [](const OccHandle<V3d_View>& view, QWidget* parent) {
        return new QWidgetOccView(view, parent);
    };
    return fn;
}

void IWidgetOccView::setCreator(IWidgetOccView::Creator fn)
{
    getWidgetOccViewCreator() = std::move(fn);
}

IWidgetOccView* IWidgetOccView::create(const OccHandle<V3d_View>& view, QWidget* parent)
{
    const auto& fn = getWidgetOccViewCreator();
    return fn(view, parent);
}

#if OCC_VERSION_HEX >= 0x070600

// Defined in widget_occ_view_impl.cpp
OccHandle<Graphic3d_GraphicDriver> QOpenGLWidgetOccView_createCompatibleGraphicsDriver();

QOpenGLWidgetOccView::QOpenGLWidgetOccView(const OccHandle<V3d_View>& view, QWidget* parent)
    : QOpenGLWidget(parent),
      IWidgetOccView(view)
{
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    this->setMouseTracking(true);
    this->setBackgroundRole(QPalette::NoRole);

    this->setUpdatesEnabled(true);
    this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}

QOpenGLWidgetOccView::~QOpenGLWidgetOccView()
{
}

void QOpenGLWidgetOccView::redraw()
{
    this->update();
}

QOpenGLWidgetOccView* QOpenGLWidgetOccView::create(const OccHandle<V3d_View>& view, QWidget* parent)
{
    return new QOpenGLWidgetOccView(view, parent);
}

OccHandle<Graphic3d_GraphicDriver> QOpenGLWidgetOccView::createCompatibleGraphicsDriver()
{
    return QOpenGLWidgetOccView_createCompatibleGraphicsDriver();
}

void QOpenGLWidgetOccView::initializeGL()
{
    auto driver = OccHandle<OpenGl_GraphicDriver>::DownCast(this->v3dView()->Viewer()->Driver());
    QtOpenGlUtils::setCapsFromSurfaceFormat(driver->ChangeOptions(), this->format());

    auto nativeWin = (Aspect_Drawable)this->effectiveWinId();
    const QRect wrect = this->rect();
    const Graphic3d_Vec2i viewSize(wrect.right() - wrect.left(), wrect.bottom() - wrect.top());

    if (!QtOpenGlUtils::initializeGlWindow(this->v3dView(), nativeWin, viewSize, this->devicePixelRatioF())) {
        Message::SendFail() << "OpenGl_Context is unable to wrap OpenGL context";
        return;
    }

    // NOTE
    // OpenCascade's MSAA path uses multisample textures and GLSL "sampler2DMS" which require either
    // OpenGL 3.2(GLSL 1.50) or the extension GL_ARB_texture_multisample. Without these, the Fragment
    // Shader[occt_blit_msaa4_gamma] fails to compile causing FBO blit errors.
    // Therefore MSAA is enabled only when GL >= 3.2 is available
    if (driver->GetSharedContext()->IsGlGreaterEqual(3, 2))
        enableMsaa4x(this->v3dView());

    // Restore Qt framebuffer
    this->makeCurrent();
}

void QOpenGLWidgetOccView::paintGL()
{
    if (!this->v3dView()->Window())
        return;

    const double devPixelRatioOld = this->v3dView()->Window()->DevicePixelRatio();
    auto nativeWin = (Aspect_Drawable)this->effectiveWinId();
    if (this->v3dView()->Window()->NativeHandle() != QtOpenGlUtils::glNativeWindow(nativeWin)) {
        // Workaround window recreation done by Qt on monitor (QScreen) disconnection
        Message::SendWarning() << "Native window handle has changed by QOpenGLWidget!";
        this->initializeGL();
    }
    else if (this->devicePixelRatioF() != devPixelRatioOld) {
        this->initializeGL();
    }

    // Wrap FBO created by QOpenGLFramebufferObject
    if (!QtOpenGlUtils::initializeGlFramebufferObject(this->v3dView())) {
        Message::SendWarning() << "Default FBO wrapper creation failed";
        return;
    }

    // Reset global GL state from Qt before redrawing OCCT
    QtOpenGlUtils::resetGlStateBeforeOcct(this->v3dView());

    // Flush pending input events and redraw the viewer
    //Handle(V3d_View) aView = !myFocusView.IsNull() ? myFocusView : myView;
    //aView->InvalidateImmediate();
    //AIS_ViewController::FlushViewEvents(myContext, aView, true);
    this->v3dView()->Redraw();

    // Reset global GL state after OCCT before redrawing Qt
    QtOpenGlUtils::resetGlStateAfterOcct(this->v3dView());
}

#endif // OCC_VERSION_HEX >= 0x070600


QWidgetOccView::QWidgetOccView(const OccHandle<V3d_View>& view, QWidget* parent)
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

// Defined in widget_occ_view.cpp
OccHandle<Graphic3d_GraphicDriver> QWidgetOccView_createCompatibleGraphicsDriver();
OccHandle<Graphic3d_GraphicDriver> QWidgetOccView::createCompatibleGraphicsDriver()
{
    return QWidgetOccView_createCompatibleGraphicsDriver();
}

void QWidgetOccView::redraw()
{
    this->v3dView()->Redraw();
}

QWidgetOccView* QWidgetOccView::create(const OccHandle<V3d_View>& view, QWidget* parent)
{
    return new QWidgetOccView(view, parent);
}

void QWidgetOccView::showEvent(QShowEvent*)
{
    if (this->v3dView()->Window().IsNull()) {
        auto hWnd = makeOccHandle<OcctWindow>(this);
        this->v3dView()->SetWindow(hWnd);
        if (!hWnd->IsMapped())
            hWnd->Map();

        this->v3dView()->MustBeResized();
        enableMsaa4x(this->v3dView());
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
