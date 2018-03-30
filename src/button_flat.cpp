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

#include "button_flat.h"
#include "theme.h"

#include <QtCore/QSignalBlocker>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>

namespace Mayo {

ButtonFlat::ButtonFlat(QWidget *parent)
    : QWidget(parent),
      m_iconSize(16, 16),
      m_hoverBrush(mayoTheme()->color(Theme::Color::FlatHover))
{
    this->setBackgroundBrush(mayoTheme()->color(Theme::Color::FlatBackground));
    this->setFixedSize(24, 24);
}

bool ButtonFlat::isCheckable() const
{
    return m_isCheckable;
}

void ButtonFlat::setCheckable(bool on)
{
    m_isCheckable = on;
}

bool ButtonFlat::isChecked() const
{
    return m_isCheckable ? m_isChecked : false;
}

void ButtonFlat::setChecked(bool on)
{
    if (m_isCheckable) {
        if (m_defaultAction != nullptr)
            m_defaultAction->setChecked(on);
        else
            m_isChecked = on;
        this->update();
    }
}

const QIcon &ButtonFlat::icon() const
{
    return m_icon;
}

void ButtonFlat::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

const QSize &ButtonFlat::iconSize() const
{
    return m_iconSize;
}

void ButtonFlat::setIconSize(const QSize &size)
{
    m_iconSize = size;
}

QAction *ButtonFlat::defaultAction() const
{
    return m_defaultAction;
}

void ButtonFlat::setDefaultAction(QAction *action)
{
    m_defaultAction = action;
    this->syncToAction();
    QObject::connect(action, &QAction::triggered, this, &ButtonFlat::clicked);
    QObject::connect(action, &QAction::changed, this, &ButtonFlat::syncToAction);
}

const QBrush &ButtonFlat::hoverBrush() const
{
    return m_hoverBrush;
}

void ButtonFlat::setHoverBrush(const QBrush &brush)
{
    m_hoverBrush = brush;
    this->update();
}

const QBrush &ButtonFlat::backgroundBrush() const
{
    return this->palette().button();
}

void ButtonFlat::setBackgroundBrush(const QBrush &brush)
{
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Button, brush);
    this->setPalette(pal);
}

void ButtonFlat::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QRect frame = this->frameGeometry();
    const QRect surface(0, 0, frame.width(), frame.height());
    const QPoint mousePos = this->mapFromGlobal(QCursor::pos());
    const bool isEnabled = this->isEnabled();

    if (isEnabled && (surface.contains(mousePos) || this->isChecked()))
        painter.fillRect(surface, m_hoverBrush);
    else
        painter.fillRect(surface, this->backgroundBrush());
    const QRect iconRect(
                (surface.width() - m_iconSize.width()) / 2,
                (surface.height() - m_iconSize.height()) / 2,
                m_iconSize.width(),
                m_iconSize.height());
    const QIcon::Mode iconMode = isEnabled ? QIcon::Normal : QIcon::Disabled;
    m_icon.paint(&painter, iconRect, Qt::AlignCenter, iconMode);
}

void ButtonFlat::enterEvent(QEvent* event)
{
    this->update();
    QWidget::enterEvent(event);
}

void ButtonFlat::leaveEvent(QEvent* event)
{
    this->update();
    QWidget::leaveEvent(event);
}

void ButtonFlat::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
        if (m_defaultAction != nullptr) {
            QSignalBlocker sigBlock(this); Q_UNUSED(sigBlock);
            m_defaultAction->trigger();
        }
        if (m_isCheckable)
            this->setChecked(!m_isChecked);
    }
    QWidget::mouseReleaseEvent(event);
}

void ButtonFlat::syncToAction()
{
    this->setIcon(m_defaultAction->icon());
    this->setToolTip(m_defaultAction->toolTip());
    this->setEnabled(m_defaultAction->isEnabled());
    this->setVisible(m_defaultAction->isVisible());
    this->setChecked(m_defaultAction->isChecked());
    this->setCheckable(m_defaultAction->isCheckable());
}

} // namespace Mayo
