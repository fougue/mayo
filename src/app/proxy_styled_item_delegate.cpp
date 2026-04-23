/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "proxy_styled_item_delegate.h"

namespace Mayo {

ProxyStyledItemDelegate::ProxyStyledItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent),
      m_sourceDelegate(nullptr)
{
}

ProxyStyledItemDelegate::ProxyStyledItemDelegate(QStyledItemDelegate* srcDelegate, QObject* parent)
    : QStyledItemDelegate(parent),
      m_sourceDelegate(srcDelegate)
{
}

QStyledItemDelegate* ProxyStyledItemDelegate::sourceDelegate() const
{
    return m_sourceDelegate;
}

void ProxyStyledItemDelegate::setSourceDelegate(QStyledItemDelegate* srcDelegate)
{
    m_sourceDelegate = srcDelegate;
}

void ProxyStyledItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const
{
    if (m_sourceDelegate)
        m_sourceDelegate->paint(painter, option, index);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

QSize ProxyStyledItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (m_sourceDelegate)
        return m_sourceDelegate->sizeHint(option, index);
    else
        return QStyledItemDelegate::sizeHint(option, index);
}

QString ProxyStyledItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    if (m_sourceDelegate)
        return m_sourceDelegate->displayText(value, locale);
    else
        return QStyledItemDelegate::displayText(value, locale);
}

QWidget* ProxyStyledItemDelegate::createEditor(
        QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const
{
    if (m_sourceDelegate)
        return m_sourceDelegate->createEditor(parent, option, index);
    else
        return QStyledItemDelegate::createEditor(parent, option, index);
}

void ProxyStyledItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (m_sourceDelegate)
        m_sourceDelegate->setEditorData(editor, index);
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void ProxyStyledItemDelegate::setModelData(
        QWidget* editor,
        QAbstractItemModel* model,
        const QModelIndex& index
    ) const
{
    if (m_sourceDelegate)
        m_sourceDelegate->setModelData(editor, model, index);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}

void ProxyStyledItemDelegate::updateEditorGeometry(
        QWidget* editor,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const
{
    if (m_sourceDelegate)
        m_sourceDelegate->updateEditorGeometry(editor, option, index);
    else
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

} // namespace Mayo
