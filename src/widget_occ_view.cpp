/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "widget_occ_view.h"

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

WidgetOccView::WidgetOccView(QWidget *parent)
    : QWidget(parent)
{ }

WidgetOccView::WidgetOccView(const Handle_V3d_Viewer& viewer, QWidget* parent)
    : QWidget(parent),
      m_viewer(viewer)
{ }

const Handle_V3d_Viewer &WidgetOccView::occV3dViewer() const
{
    return m_viewer;
}

void WidgetOccView::setOccV3dViewer(const Handle_V3d_Viewer &viewer)
{
    Q_ASSERT(m_viewer.IsNull());
    m_viewer = viewer;
}

const Handle_V3d_View& WidgetOccView::occV3dView() const
{
    return m_view;
}

//! Hack for Qt 4.5.x
QPaintEngine* WidgetOccView::paintEngine() const
{
    return nullptr;
}

//! Force a redraw of the view
void WidgetOccView::redraw()
{
    if (!m_view.IsNull()) {
        if (m_needsResize)
            m_view->MustBeResized();
        else
            m_view->Redraw();
    }
    m_needsResize = false;
}

void WidgetOccView::fitAll()
{
    if (!m_view.IsNull()) {
        m_view->ZFitAll();
        m_view->FitAll();
    }
}

//! Reimplemented from QWidget::paintEvent()
void WidgetOccView::paintEvent(QPaintEvent* /*event*/)
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
void WidgetOccView::resizeEvent(QResizeEvent* /*event*/)
{
    m_needsResize = true;
}

void WidgetOccView::initialize()
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

        Handle_Aspect_Window hWnd = new OcctWindow(this);
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
                    0.075,
                    V3d_ZBUFFER);

        m_view->MustBeResized();
        m_isInitialized = true;
        m_needsResize = true;
    }
}

} // namespace Mayo
