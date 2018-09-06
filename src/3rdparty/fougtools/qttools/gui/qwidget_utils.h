/****************************************************************************
**  FougTools
**  Copyright Fougue (30 Mar. 2015)
**  contact@fougue.pro
**
** This software is a computer program whose purpose is to provide utility
** tools for the C++ language and the Qt toolkit.
**
** This software is governed by the CeCILL-C license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-C
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
****************************************************************************/

#pragma once

#include "gui.h"
#include <QtCore/QPair>
#include <QtCore/QPoint>
#include <QMessageBox>

class QAbstractScrollArea;
class QDialog;
class QMenu;
class QWidget;

namespace qtgui {

class QTTOOLS_GUI_EXPORT QWidgetUtils {
public:
    template<typename PARENT_WIDGET>
    static PARENT_WIDGET* findFirstParentWidget(QWidget* widget);

    template<typename PARENT_WIDGET>
    static PARENT_WIDGET* findLastParentWidget(QWidget* widget);

    static void wrapWidgetInDialog(QWidget* widget, QDialog* dialog);
    static void addContentsWidget(
            QWidget* containerWidget, QWidget* contentsWidget);

    static QPoint globalPos(const QWidget* widget, Qt::Corner widgetCorner);
    static void moveWidgetRightTo(QWidget* widget, const QWidget* nextTo, int margin = 0);
    static void moveWidgetLeftTo(QWidget* widget, const QWidget* nextTo, int margin = 0);

    static QPair<int, int> horizAndVertScrollValue(const QAbstractScrollArea* area);
    static void setHorizAndVertScrollValue(
            QAbstractScrollArea* area, const QPair<int, int>& values);

    static void asyncDialogExec(QDialog* dialog);
    static void asyncMenuExec(QMenu* menu, const QPoint& pos = QCursor::pos());
    static QMessageBox* asyncMsgBoxInfo(
            QWidget* parent,
            const QString& title,
            const QString& text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok);
    static QMessageBox* asyncMsgBoxWarning(
            QWidget* parent,
            const QString& title,
            const QString& text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok);
    static QMessageBox* asyncMsgBoxCritical(
            QWidget* parent,
            const QString& title,
            const QString& text,
            QMessageBox::StandardButtons buttons = QMessageBox::Ok);
};

} // namespace qtgui

// --
// -- Implementation
// --

// QtWidgets
#include <QWidget>

namespace qtgui {

//! Searches up in the direct parents of \p widget the first ancestor being of
//! type \c PARENT_WIDGET
template<typename PARENT_WIDGET>
PARENT_WIDGET* QWidgetUtils::findFirstParentWidget(QWidget* widget)
{
    PARENT_WIDGET* foundParentWidget = nullptr;
    QWidget* iteratorWidget = widget;
    while (iteratorWidget != nullptr && foundParentWidget == nullptr) {
        iteratorWidget = iteratorWidget->parentWidget();
        foundParentWidget = qobject_cast<PARENT_WIDGET*>(iteratorWidget);
    }
    return foundParentWidget;
}

//! Searches up in the direct parents of \p widget the last ancestor being of
//! type \c PARENT_WIDGET
template<typename PARENT_WIDGET>
PARENT_WIDGET* QWidgetUtils::findLastParentWidget(QWidget* widget)
{
    PARENT_WIDGET* foundParentWidget = nullptr;
    QWidget* iteratorWidget = widget;
    while (iteratorWidget != nullptr) {
        iteratorWidget = iteratorWidget->parentWidget();
        PARENT_WIDGET* currParentWidget = qobject_cast<PARENT_WIDGET*>(iteratorWidget);
        if (currParentWidget != nullptr)
            foundParentWidget = currParentWidget;
    }
    return foundParentWidget;
}

} // namespace qtgui
