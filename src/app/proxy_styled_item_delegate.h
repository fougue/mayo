/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QStyledItemDelegate>

namespace Mayo {

// Convenience class that simplifies dynamically overriding of QStyledItemDelegate
// QStyledItemDelegate protected functions cannot be overriden through proxy technique, this is a
// limitation that applies to :
//     - QStyledItemDelegate::initStyleOption()
//     - QStyledItemDelegate::eventFilter()
//     - QStyledItemDelegate::editorEvent()
class ProxyStyledItemDelegate : public QStyledItemDelegate {
public:
    ProxyStyledItemDelegate(QObject* parent = nullptr);
    ProxyStyledItemDelegate(QStyledItemDelegate* srcDelegate, QObject* parent = nullptr);

    QStyledItemDelegate* sourceDelegate() const;
    void setSourceDelegate(QStyledItemDelegate* srcDelegate);

    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
    QSize sizeHint(
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
    QString displayText(
            const QVariant& value,
            const QLocale& locale) const override;

    QWidget* createEditor(
            QWidget* parent,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;
    void setEditorData(
            QWidget* editor, const QModelIndex& index) const override;
    void setModelData(
            QWidget* editor,
            QAbstractItemModel* model,
            const QModelIndex& index) const override;
    void updateEditorGeometry(
            QWidget* editor,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const override;

private:
    QStyledItemDelegate* m_sourceDelegate;
};

} // namespace Mayo
