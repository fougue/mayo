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

#include "proxy_styled_item_delegate.h"

namespace qtgui {

/*!
 * \class ProxyStyledItemDelegate
 * \brief Convenience class that simplifies dynamically overriding QStyledItemDelegate
 *
 * QStyledItemDelegate protected functions cannot be overriden through proxy
 * technique, this is a limitation that applies to :
 *   \li QStyledItemDelegate::initStyleOption()
 *   \li QStyledItemDelegate::eventFilter()
 *   \li QStyledItemDelegate::editorEvent()
 *
 * \headerfile proxy_styled_item_delegate.h <qttools/gui/proxy_styled_item_delegate.h>
 * \ingroup qttools_gui
 *
 */

ProxyStyledItemDelegate::ProxyStyledItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_sourceDelegate(NULL)
{

}

ProxyStyledItemDelegate::ProxyStyledItemDelegate(
        QStyledItemDelegate *srcDelegate,
        QObject* parent)
    : QStyledItemDelegate(parent),
      m_sourceDelegate(srcDelegate)
{
}

QStyledItemDelegate *ProxyStyledItemDelegate::sourceDelegate() const
{
    return m_sourceDelegate;
}

void ProxyStyledItemDelegate::setSourceDelegate(QStyledItemDelegate *srcDelegate)
{
    m_sourceDelegate = srcDelegate;
}

void ProxyStyledItemDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        m_sourceDelegate->paint(painter, option, index);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

QSize ProxyStyledItemDelegate::sizeHint(
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        return m_sourceDelegate->sizeHint(option, index);
    return QStyledItemDelegate::sizeHint(option, index);
}

QString ProxyStyledItemDelegate::displayText(
        const QVariant &value, const QLocale &locale) const
{
    if (m_sourceDelegate != NULL)
        return m_sourceDelegate->displayText(value, locale);
    return QStyledItemDelegate::displayText(value, locale);
}

QWidget *ProxyStyledItemDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        return m_sourceDelegate->createEditor(parent, option, index);
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ProxyStyledItemDelegate::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        m_sourceDelegate->setEditorData(editor, index);
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void ProxyStyledItemDelegate::setModelData(
        QWidget *editor,
        QAbstractItemModel *model,
        const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        m_sourceDelegate->setModelData(editor, model, index);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}

void ProxyStyledItemDelegate::updateEditorGeometry(
        QWidget *editor,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    if (m_sourceDelegate != NULL)
        m_sourceDelegate->updateEditorGeometry(editor, option, index);
    else
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

} // namespace qtgui
