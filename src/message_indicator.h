#pragma once

#include <QtWidgets/QWidget>

namespace Mayo {

/*! Provides an auto-shading application message
 *
 *  Use directly MessageIndicator::showMessage() instead of manually creating
 *  MessageIndicator.\n
 *  Messages are displayed at widget's bottom-left corner and during a time
 *  proportional to the length of the message.
 */
class MessageIndicator : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity USER true)

public:
    MessageIndicator(const QString& msg, QWidget* parent = nullptr);

    qreal opacity() const;
    void setOpacity(qreal value);

    void run();

    static void showMessage(const QString& msg, QWidget* parent);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void runInternal();

    const QString m_message;
    QRectF m_messageRect;
    qreal m_opacity = 1.;
};

} // namespace Mayo
