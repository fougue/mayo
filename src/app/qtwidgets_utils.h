/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtGui/QCursor>
#include <QtWidgets/QMessageBox>
class QMenu;
class QScreen;
class QWidget;

namespace Mayo {

// Provides a collection of tools for the QtWidgets module
class QtWidgetsUtils {
public:
    static QScreen* screen(const QWidget* widget);
    static int screenPixelWidth(double screenRatio, const QWidget* parentWidget = nullptr);
    static int screenPixelHeight(double screenRatio, const QWidget* parentWidget = nullptr);
    static QSize screenPixelSize(double widthRatio, double heightRatio, const QWidget* parentWidget = nullptr);

    // Add 'contentsWidget' to 'containerWidget'
    // If 'containerWidget' is empty then a QBoxLayout is created to receive 'contentsWidget'
    static void addContentsWidget(QWidget* containerWidget, QWidget* contentsWidget);

    // Executes 'dialog' asynchronously(non blocking)
    // The dialog object is automatically destroyed when execution is finished
    static void asyncDialogExec(QDialog* dialog);

    // Executes(popup) 'menu' asynchronously(non blocking)
    // The menu object is automatically destroyed when execution is finished
    static void asyncMenuExec(QMenu* menu, const QPoint& pos = QCursor::pos());

    static QMessageBox* asyncMsgBoxInfo(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok
    );
    static QMessageBox* asyncMsgBoxWarning(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok
    );
    static QMessageBox* asyncMsgBoxCritical(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons = QMessageBox::Ok
    );

    // Move position of 'widget' so it's displayed stuck to the right of 'nextTo'
    static void moveWidgetRightTo(QWidget* widget, const QWidget* nextTo, int margin);
    // Move position of 'widget' so it's displayed stuck to the left of 'nextTo'
    static void moveWidgetLeftTo(QWidget* widget, const QWidget* nextTo, int margin = 0);

    static void collapseWidget(QWidget* widget, bool on);
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Qt6 QWidget::enterEvent(QEnterEvent*)
using QWidgetEnterEvent = QEnterEvent;
#else
// Qt5 QWidget::enterEvent(QEvent*)
using QWidgetEnterEvent = QEvent;
#endif

} // namespace Mayo
