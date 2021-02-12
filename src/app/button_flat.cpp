/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "button_flat.h"
#include "theme.h"

#include <QtCore/QSignalBlocker>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAction>

#include <QtCore/QtDebug>

namespace Mayo {

ButtonFlat::ButtonFlat(QWidget* parent)
    : QWidget(parent),
      m_iconSize(16, 16),
      m_hoverBrush(mayoTheme()->color(Theme::Color::ButtonFlat_Hover)),
      m_checkedBrush(mayoTheme()->color(Theme::Color::ButtonFlat_Checked))
{
    this->setBackgroundBrush(mayoTheme()->color(Theme::Color::ButtonFlat_Background));
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
    if (!m_isCheckable)
        return;

    if (m_defaultAction != nullptr)
        m_defaultAction->setChecked(on);
    else
        m_isChecked = on;

    this->update();
}

const QIcon &ButtonFlat::icon() const
{
    return m_icon;
}

void ButtonFlat::setIcon(const QIcon& icon)
{
    m_icon = icon;
}

const QSize& ButtonFlat::iconSize() const
{
    return m_iconSize;
}

void ButtonFlat::setIconSize(const QSize& size)
{
    m_iconSize = size;
}

QAction *ButtonFlat::defaultAction() const
{
    return m_defaultAction;
}

void ButtonFlat::setDefaultAction(QAction* action)
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

void ButtonFlat::setHoverBrush(const QBrush& brush)
{
    m_hoverBrush = brush;
    this->update();
}

const QBrush &ButtonFlat::checkedBrush() const
{
    return m_checkedBrush;
}

void ButtonFlat::setCheckedBrush(const QBrush& brush)
{
    m_checkedBrush = brush;
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
    //const QPoint mousePos = this->mapFromGlobal(QCursor::pos());
    const bool isEnabled = this->isEnabled();

    if (isEnabled && this->isChecked())
        painter.fillRect(surface, m_checkedBrush);
    else if (isEnabled && m_isMouseHover)
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
    m_isMouseHover = true;
    this->update();
    QWidget::enterEvent(event);
}

void ButtonFlat::leaveEvent(QEvent* event)
{
    m_isMouseHover = false;
    this->update();
    QWidget::leaveEvent(event);
}

void ButtonFlat::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_defaultAction != nullptr) {
            QSignalBlocker sigBlock(this); Q_UNUSED(sigBlock);
            m_defaultAction->trigger();
        }

        if (m_isCheckable)
            this->setChecked(!m_isChecked);
        emit clicked();
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
