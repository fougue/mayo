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

// QtWidgets
#include <QStyledItemDelegate>

namespace qtgui {

class QTTOOLS_GUI_EXPORT ProxyStyledItemDelegate : public QStyledItemDelegate
{
public:
    ProxyStyledItemDelegate(QObject* parent = NULL);
    ProxyStyledItemDelegate(QStyledItemDelegate* srcDelegate, QObject* parent = NULL);

    QStyledItemDelegate* sourceDelegate() const;
    void setSourceDelegate(QStyledItemDelegate* srcDelegate);

    void paint(
            QPainter *painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const Q_DECL_OVERRIDE;
    QString displayText(
            const QVariant &value,
            const QLocale &locale) const Q_DECL_OVERRIDE;

    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(
            QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(
            QWidget *editor,
            QAbstractItemModel *model,
            const QModelIndex &index) const Q_DECL_OVERRIDE;
    void updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QStyledItemDelegate* m_sourceDelegate;
};

} // namespace qtgui
