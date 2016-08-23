#include "qt_occ_view.h"

#include "occt_window.h"

#if defined(Q_OS_WIN32)
# include <windows.h>
#endif

#include <Graphic3d_GraphicDriver.hxx>

#include <QtCore/QtDebug>

#include <QApplication>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <GL/gl.h>
#include <GL/glu.h>

#if defined(Q_OS_WIN32)
# include <WNT_Window.hxx>
#elif defined(Q_OS_MAC) && !defined(MACOSX_USE_GLX)
# include <Cocoa_Window.hxx>
#else
# include <Aspect_DisplayConnection.hxx>
# include <Xw_Window.hxx> // OpenCascade
# include <X11/X.h>
#endif

namespace Mayo {

QtOccView::QtOccView(QWidget *parent)
    : QWidget(parent)
{ }

QtOccView::QtOccView(const Handle_V3d_Viewer& viewer, QWidget* parent)
    : QWidget(parent),
      m_viewer(viewer)
{ }

const Handle_V3d_Viewer &QtOccView::v3dViewer() const
{
    return m_viewer;
}

void QtOccView::setV3dViewer(const Handle_V3d_Viewer &viewer)
{
    Q_ASSERT(m_viewer.IsNull());
    m_viewer = viewer;
}

const Handle_V3d_View& QtOccView::v3dView() const
{
    return m_view;
}

//! Hack for Qt 4.5.x
QPaintEngine* QtOccView::paintEngine() const
{
    return nullptr;
}

//! Force a redraw of the view
void QtOccView::redraw()
{
    if (!m_view.IsNull()) {
        if (m_needsResize)
            m_view->MustBeResized();
        else
            m_view->Redraw();
    }
    m_needsResize = false;
}

void QtOccView::fitAll()
{
    if (!m_view.IsNull()) {
        m_view->ZFitAll();
        m_view->FitAll();
    }
}

//! Reimplemented from QWidget::paintEvent()
void QtOccView::paintEvent(QPaintEvent* /*event*/)
{
    initialize();
    if (!m_viewer.IsNull())
        this->redraw();
}

/*! Reimplemented from QWidget::resizeEvent()
 *
 *  Called when the widget needs to resize itself, but seeing as a paint event
 *  always follows a resize event, we'll move the work into the paint event
 */
void QtOccView::resizeEvent(QResizeEvent* /*event*/)
{
    m_needsResize = true;
}

void QtOccView::initialize()
{
    if (!m_isInitialized) {
        this->setMouseTracking(true);

        // Avoid Qt background clears to improve resizing speed
        this->setAutoFillBackground(false);
        this->setAttribute(Qt::WA_NoSystemBackground);
        this->setAttribute(Qt::WA_PaintOnScreen);
        this->setAttribute(Qt::WA_OpaquePaintEvent);
        this->setAttribute(Qt::WA_NativeWindow);

        m_view = m_viewer->CreateView();

        Handle_OcctWindow hWnd = new OcctWindow(this);
        m_view->SetWindow(hWnd);
        if (!hWnd->IsMapped())
            hWnd->Map();

        m_view->SetBgGradientColors(
                    Quantity_Color(0.5, 0.58, 1., Quantity_TOC_RGB),
                    Quantity_NOC_WHITE,
                    Aspect_GFM_VER);

        m_view->TriedronDisplay(
                    Aspect_TOTP_LEFT_LOWER,
                    Quantity_NOC_GRAY50,
                    0.1,
                    V3d_ZBUFFER);

        m_view->MustBeResized();
        m_isInitialized = true;
        m_needsResize = true;
    }
}

} // namespace Mayo
