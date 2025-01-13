/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qtwidgets_utils.h"
#include "qtgui_utils.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidget>

namespace Mayo {

QScreen* QtWidgetsUtils::screen(const QWidget* widget)
{
    const QWindow* window = widget ? widget->windowHandle() : nullptr;
    return window ? window->screen() : QGuiApplication::primaryScreen();
}

int QtWidgetsUtils::screenPixelWidth(double screenRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelWidth(screenRatio, QtWidgetsUtils::screen(parentWidget));
}

int QtWidgetsUtils::screenPixelHeight(double screenRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelHeight(screenRatio, QtWidgetsUtils::screen(parentWidget));
}

QSize QtWidgetsUtils::screenPixelSize(double widthRatio, double heightRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelSize(widthRatio, heightRatio, QtWidgetsUtils::screen(parentWidget));
}

void QtWidgetsUtils::addContentsWidget(QWidget* containerWidget, QWidget* contentsWidget)
{
    if (containerWidget && contentsWidget) {
        if (!containerWidget->layout()) {
            containerWidget->setLayout(new QVBoxLayout);
            containerWidget->layout()->setContentsMargins(0, 0, 0, 0);
        }

        contentsWidget->setParent(containerWidget);
        containerWidget->layout()->addWidget(contentsWidget);
    }
}

void QtWidgetsUtils::asyncMenuExec(QMenu* menu, const QPoint& pos)
{
    if (menu) {
        QObject::connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
        menu->popup(pos);
    }
}

void QtWidgetsUtils::asyncDialogExec(QDialog* dialog)
{
    if (!dialog)
        return;

    QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater, Qt::UniqueConnection);
    dialog->show();
}

QMessageBox* QtWidgetsUtils::asyncMsgBoxInfo(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons
    )
{
    auto msgBox = new QMessageBox(QMessageBox::Information, title, text, buttons, parent);
    QtWidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

QMessageBox* QtWidgetsUtils::asyncMsgBoxWarning(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons
    )
{
    auto msgBox = new QMessageBox(QMessageBox::Warning, title, text, buttons, parent);
    QtWidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

QMessageBox* QtWidgetsUtils::asyncMsgBoxCritical(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons
    )
{
    auto msgBox = new QMessageBox(QMessageBox::Critical, title, text, buttons, parent);
    QtWidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

void QtWidgetsUtils::moveWidgetRightTo(QWidget* widget, const QWidget* nextTo, int margin)
{
    const QRect frameGeom = nextTo->frameGeometry();
    widget->move(nextTo->mapToParent(QPoint(frameGeom.width() + margin, 0)));
}

void QtWidgetsUtils::moveWidgetLeftTo(QWidget* widget, const QWidget* nextTo, int margin)
{
    const QRect frameGeom = widget->frameGeometry();
    widget->move(nextTo->mapToParent(QPoint(-frameGeom.width() - margin, 0)));
}

void QtWidgetsUtils::collapseWidget(QWidget *widget, bool on)
{
    widget->setMaximumHeight(on ? 0 : 16777215/*Qt_defaultMaximumWidth*/);
}

} // namespace Mayo
