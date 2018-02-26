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

#include "button_view3d.h"

#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

namespace Mayo {

ButtonView3d::ButtonView3d(QWidget *parent)
    : QWidget(parent),
      m_iconSize(24, 24),
      m_backgroundColor(this->palette().color(QPalette::Button)),
      m_hoverColor(65, 200, 250)
{ }

const QIcon &ButtonView3d::icon() const
{
    return m_icon;
}

void ButtonView3d::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

const QSize &ButtonView3d::iconSize() const
{
    return m_iconSize;
}

void ButtonView3d::setIconSize(const QSize &size)
{
    m_iconSize = size;
}

void ButtonView3d::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QRect frame = this->frameGeometry();
    const QRect surface(0, 0, frame.width(), frame.height());
    const QPoint mousePos = this->mapFromGlobal(QCursor::pos());

    if (surface.contains(mousePos))
        painter.fillRect(surface, m_hoverColor);
    else
        painter.fillRect(surface, m_backgroundColor);
    const QRect iconRect(
                (surface.width() - m_iconSize.width()) / 2,
                (surface.height() - m_iconSize.height()) / 2,
                m_iconSize.width(),
                m_iconSize.height());
    m_icon.paint(&painter, iconRect, Qt::AlignCenter);
}

void ButtonView3d::enterEvent(QEvent* event)
{
    this->update();
    QWidget::enterEvent(event);
}

void ButtonView3d::leaveEvent(QEvent* event)
{
    this->update();
    QWidget::leaveEvent(event);
}

void ButtonView3d::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QWidget::mouseReleaseEvent(event);
}

} // namespace Mayo
