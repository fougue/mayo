/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "proxy_styled_item_delegate.h"

namespace Mayo {

ProxyStyledItemDelegate::ProxyStyledItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

ProxyStyledItemDelegate::ProxyStyledItemDelegate(QAbstractItemDelegate* srcDelegate, QObject* parent)
    : QStyledItemDelegate(parent)
{
    this->setSourceDelegate(srcDelegate);
}

QAbstractItemDelegate* ProxyStyledItemDelegate::sourceDelegate() const
{
    return m_sourceDelegate;
}

void ProxyStyledItemDelegate::setSourceDelegate(QAbstractItemDelegate* srcDelegate)
{
    if (m_sourceDelegate) {
        QObject::disconnect(
            m_sourceDelegate, &QAbstractItemDelegate::commitData,
            this, &QAbstractItemDelegate::commitData
        );
        QObject::disconnect(
            m_sourceDelegate, &QAbstractItemDelegate::closeEditor,
            this, &QAbstractItemDelegate::closeEditor
        );
    }

    m_sourceDelegate = srcDelegate;
    if (m_sourceDelegate) {
        QObject::connect(
            m_sourceDelegate, &QAbstractItemDelegate::commitData,
            this, &QAbstractItemDelegate::commitData
        );
        QObject::connect(
            m_sourceDelegate, &QAbstractItemDelegate::closeEditor,
            this, &QAbstractItemDelegate::closeEditor
        );
    }
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
    auto srcStyledDelegate = qobject_cast<QStyledItemDelegate*>(m_sourceDelegate);
    if (srcStyledDelegate)
        return srcStyledDelegate->displayText(value, locale);
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
