/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_message_indicator.h"
#include "qtgui_utils.h"
#include "theme.h"

#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QFontMetricsF>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>

namespace Mayo {

WidgetMessageIndicator::WidgetMessageIndicator(const QString& msg, QWidget* parent)
    : QWidget(parent),
      m_message(msg),
      m_messageRect(QFontMetrics(QtGuiUtils::FontChange(this->font()).bold()).boundingRect(msg))
{
    if (parent) {
        const qreal rectWidth = m_messageRect.width() + 20;
        const qreal rectHeight = m_messageRect.height() + 10;
        this->setGeometry(QRect(5, parent->height() - rectHeight - 5, rectWidth, rectHeight));
        parent->installEventFilter(this);
    }
}

qreal WidgetMessageIndicator::opacity() const
{
    return m_opacity;
}

void WidgetMessageIndicator::setOpacity(qreal value)
{
    m_opacity = value;
    this->update();
}

void WidgetMessageIndicator::run()
{
    this->show();
    const int duration = 1500 + m_message.length() * 60;
    QTimer::singleShot(duration, this, &WidgetMessageIndicator::runInternal);
}

void WidgetMessageIndicator::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);
    p.setOpacity(m_opacity);

    const QRectF& msgRect = m_messageRect;
    const QRectF boxRect(0, 0, msgRect.width() + 18, msgRect.height() + 8);
    p.fillRect(boxRect, m_backgroundColor);

    p.setFont(QtGuiUtils::FontChange(this->font()).bold());
    const QRectF textRect(9, 4, msgRect.width() + 4, msgRect.height());
    p.setPen(m_textColor);
    p.drawText(textRect, m_message);
}

void WidgetMessageIndicator::runInternal()
{
    auto anim = new QPropertyAnimation(this, "opacity", this);
    anim->setDuration(200);
    anim->setStartValue(1.);
    anim->setEndValue(0.);
    QObject::connect(anim, &QAbstractAnimation::finished, this, &QObject::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WidgetMessageIndicator::showInfo(const QString& msg, QWidget* parent)
{
    auto widget = new WidgetMessageIndicator(msg, parent);
    widget->setTextColor(mayoTheme()->color(Theme::Color::MessageIndicator_InfoText));
    widget->setBackgroundColor(mayoTheme()->color(Theme::Color::MessageIndicator_InfoBackground));
    widget->run();
}

void WidgetMessageIndicator::showError(const QString &msg, QWidget *parent)
{
    auto widget = new WidgetMessageIndicator(msg, parent);
    widget->setTextColor(mayoTheme()->color(Theme::Color::MessageIndicator_ErrorText));
    widget->setBackgroundColor(mayoTheme()->color(Theme::Color::MessageIndicator_ErrorBackground));
    widget->run();
}

bool WidgetMessageIndicator::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->parentWidget() && event->type() == QEvent::Resize) {
        auto eventResize = static_cast<const QResizeEvent*>(event);
        if (eventResize->size().height() != eventResize->oldSize().height()) {
            const qreal rectHeight = m_messageRect.height() + 10;
            this->move(5, eventResize->size().height() - rectHeight - 5);
            return true;
        }
    }
    return false;
}

} // namespace Mayo
