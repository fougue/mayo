/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_occ_view.h"
#include "occt_window.h"

#include <QtGui/QResizeEvent>

namespace Mayo {

WidgetOccView::WidgetOccView(const Handle_V3d_View& view, QWidget* parent)
    : QWidget(parent),
      m_view(view)
{
    this->setMouseTracking(true);

    // Avoid Qt background clears to improve resizing speed
    this->setAutoFillBackground(false);
    this->setAttribute(Qt::WA_NoSystemBackground);
    this->setAttribute(Qt::WA_PaintOnScreen);
    this->setBackgroundRole(QPalette::NoRole);
}

const Handle_V3d_View &WidgetOccView::v3dView() const
{
    return m_view;
}

QPaintEngine* WidgetOccView::paintEngine() const
{
    return nullptr;
}

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

} // namespace Mayo
