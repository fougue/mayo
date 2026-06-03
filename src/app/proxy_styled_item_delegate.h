/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <QtWidgets/QStyledItemDelegate>

namespace Mayo {

// Convenience class that simplifies dynamically overriding of QStyledItemDelegate
// QStyledItemDelegate protected functions cannot be overridden through proxy technique, this
// limitation applies to :
//     - QStyledItemDelegate::initStyleOption()
//     - QStyledItemDelegate::eventFilter()
//     - QStyledItemDelegate::editorEvent()
class ProxyStyledItemDelegate : public QStyledItemDelegate {
public:
    explicit ProxyStyledItemDelegate(QObject* parent = nullptr);
    explicit ProxyStyledItemDelegate(QAbstractItemDelegate* srcDelegate, QObject* parent = nullptr);

    QAbstractItemDelegate* sourceDelegate() const;
    void setSourceDelegate(QAbstractItemDelegate* srcDelegate);

    void paint(
        QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index
    ) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QString displayText(const QVariant& value, const QLocale& locale) const override;

    QWidget* createEditor(
        QWidget* parent,const QStyleOptionViewItem& option,const QModelIndex& index
    ) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(
        QWidget* editor, QAbstractItemModel* model, const QModelIndex& index
    ) const override;

    void updateEditorGeometry(
        QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index
    ) const override;

private:
    QAbstractItemDelegate* m_sourceDelegate = nullptr;
};

} // namespace Mayo
