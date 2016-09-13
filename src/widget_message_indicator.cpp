#include "widget_message_indicator.h"

#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QFontMetricsF>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>

namespace Mayo {

namespace Internal {

static QFont indicatorFont(const QFont& font)
{
    QFont indicFont(font);
    indicFont.setBold(true);
    return indicFont;
}

} // namespace Internal

WidgetMessageIndicator::WidgetMessageIndicator(const QString& msg, QWidget* parent)
    : QWidget(parent),
      m_message(msg),
      m_messageRect(QFontMetrics(Internal::indicatorFont(this->font())).boundingRect(msg))
{
    if (parent != nullptr) {
        const qreal rectWidth = m_messageRect.width() + 20;
        const qreal rectHeight = m_messageRect.height() + 10;
        this->setGeometry(
                    QRect(5, parent->height() - rectHeight - 5, rectWidth, rectHeight));
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
    QTimer::singleShot(
                1500 + m_message.length() * 60,
                this,
                &WidgetMessageIndicator::runInternal);
}

void WidgetMessageIndicator::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);
    p.setOpacity(m_opacity);

    QPainterPath boxPath;
    const QRectF boxRect(
                0, 0, m_messageRect.width() + 18, m_messageRect.height() + 8);
    boxPath.addRoundedRect(boxRect, 8, 8);
    QLinearGradient boxGrad(0, 0, 0, boxRect.height());
    boxGrad.setColorAt(0, QColor(214, 235, 255));
    boxGrad.setColorAt(1, QColor(128, 200, 255));
    p.fillPath(boxPath, boxGrad);

    p.setFont(Internal::indicatorFont(this->font()));
    const QRectF textRect(
                9, 4, m_messageRect.width() + 4, m_messageRect.height());
    p.drawText(textRect, m_message);
}

void WidgetMessageIndicator::runInternal()
{
    auto anim = new QPropertyAnimation(this, "opacity", this);
    anim->setDuration(200);
    anim->setEndValue(0.);
    QObject::connect(
                anim, &QAbstractAnimation::finished,
                this, &QObject::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WidgetMessageIndicator::showMessage(const QString& msg, QWidget* parent)
{
    (new WidgetMessageIndicator(msg, parent))->run();
}

} // namespace Mayo
