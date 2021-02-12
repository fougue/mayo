/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

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
class WidgetMessageIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity USER true)
public:
    WidgetMessageIndicator(const QString& msg, QWidget* parent = nullptr);

    qreal opacity() const;
    void setOpacity(qreal value);

    void run();

    static void showMessage(const QString& msg, QWidget* parent);

    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    void runInternal();

    const QString m_message;
    QRectF m_messageRect;
    qreal m_opacity = 1.;
};

} // namespace Mayo
