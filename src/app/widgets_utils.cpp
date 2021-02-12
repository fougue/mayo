/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widgets_utils.h"

#include "../gui/qtgui_utils.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidget>

namespace Mayo {

QScreen* WidgetsUtils::screen(const QWidget* widget)
{
    const QWindow* window = widget ? widget->windowHandle() : nullptr;
    return window ? window->screen() : QGuiApplication::primaryScreen();
}

int WidgetsUtils::screenPixelWidth(double screenRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelWidth(screenRatio, WidgetsUtils::screen(parentWidget));
}

int WidgetsUtils::screenPixelHeight(double screenRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelHeight(screenRatio, WidgetsUtils::screen(parentWidget));
}

QSize WidgetsUtils::screenPixelSize(double widthRatio, double heightRatio, const QWidget* parentWidget)
{
    return QtGuiUtils::screenPixelSize(widthRatio, heightRatio, WidgetsUtils::screen(parentWidget));
}

void WidgetsUtils::addContentsWidget(QWidget* containerWidget, QWidget* contentsWidget)
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

void WidgetsUtils::asyncMenuExec(QMenu* menu, const QPoint& pos)
{
    if (menu) {
        QObject::connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater);
        menu->popup(pos);
    }
}

void WidgetsUtils::asyncDialogExec(QDialog* dialog)
{
    if (dialog) {
        QObject::connect(
                    dialog, &QDialog::finished,
                    dialog, &QObject::deleteLater,
                    Qt::UniqueConnection);
        dialog->show();
    }
}
QMessageBox* WidgetsUtils::asyncMsgBoxInfo(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons)
{
    auto msgBox = new QMessageBox(QMessageBox::Information, title, text, buttons, parent);
    WidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

QMessageBox* WidgetsUtils::asyncMsgBoxWarning(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons)
{
    auto msgBox = new QMessageBox(QMessageBox::Warning, title, text, buttons, parent);
    WidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

QMessageBox* WidgetsUtils::asyncMsgBoxCritical(
        QWidget* parent,
        const QString& title,
        const QString& text,
        QMessageBox::StandardButtons buttons)
{
    auto msgBox = new QMessageBox(QMessageBox::Critical, title, text, buttons, parent);
    WidgetsUtils::asyncDialogExec(msgBox);
    return msgBox;
}

void WidgetsUtils::moveWidgetRightTo(QWidget* widget, const QWidget* nextTo, int margin)
{
    const QRect frameGeom = nextTo->frameGeometry();
    widget->move(nextTo->mapToParent(QPoint(frameGeom.width() + margin, 0)));
}

void WidgetsUtils::moveWidgetLeftTo(QWidget* widget, const QWidget* nextTo, int margin)
{
    const QRect frameGeom = widget->frameGeometry();
    widget->move(nextTo->mapToParent(QPoint(-frameGeom.width() - margin, 0)));
}

} // namespace Mayo
