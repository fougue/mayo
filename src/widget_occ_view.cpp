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

void WidgetOccView::resizeEvent(QResizeEvent*)
{
    m_view->MustBeResized();
}

} // namespace Mayo
