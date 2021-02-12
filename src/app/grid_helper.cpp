/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "grid_helper.h"

#include <QtCore/QEvent>
#include <QtGui/QHoverEvent>
#include <QtWidgets/QHeaderView>
#include <algorithm>

namespace Mayo {
namespace GridHelper {

void ProxyModel::setSourceModel(QAbstractItemModel* newModel)
{
    if (m_sourceModel == newModel)
        return;

    if (m_sourceModel)
        QObject::disconnect(m_sourceModel, nullptr, this, nullptr);

    m_sourceModel = newModel;
    if (newModel) {
        QObject::connect(newModel, &QAbstractItemModel::layoutAboutToBeChanged, this, [=]{
            emit this->layoutAboutToBeChanged();
        });
        QObject::connect(newModel, &QAbstractItemModel::layoutChanged, this, [=]{
            emit this->layoutChanged();
        });
        QObject::connect(
                    newModel, &QAbstractItemModel::modelAboutToBeReset,
                    this, &ProxyModel::beginResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::modelReset,
                    this, &ProxyModel::endResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::rowsAboutToBeInserted,
                    this, &ProxyModel::beginResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::rowsInserted,
                    this, &ProxyModel::endResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::rowsAboutToBeRemoved,
                    this, &ProxyModel::beginResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::rowsRemoved,
                    this, &ProxyModel::endResetModel);
        QObject::connect(
                    newModel, &QAbstractItemModel::dataChanged,
                    this, &ProxyModel::onDataChanged);
    }
}

QVariant ProxyModel::data(const QModelIndex& index, int role) const
{
    const std::optional<QModelIndex> sourceIndex = this->mapToSource(index);
    return sourceIndex ? this->sourceModel()->data(*sourceIndex, role) : QVariant();
}

Qt::ItemFlags ProxyModel::flags(const QModelIndex& index) const
{
    const std::optional<QModelIndex> sourceIndex = this->mapToSource(index);
    return sourceIndex ? this->sourceModel()->flags(*sourceIndex) : Qt::ItemFlags();
}

bool ProxyModel::hasChildren(const QModelIndex& parent) const
{
    const std::optional<QModelIndex> sourceParent = this->mapToSource(parent);
    return sourceParent ? this->sourceModel()->hasChildren(*sourceParent) : false;
}

int ProxyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    const int rows = this->sourceModel()->rowCount(QModelIndex());
    return (rows + m_columnCount - 1) / m_columnCount;
}

int ProxyModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_columnCount;
}

void ProxyModel::setColumnCount(int count)
{
    count = std::max(count, 1);
    if (count != m_columnCount) {
        m_columnCount = count;
        emit this->layoutChanged();
    }
}

QModelIndex ProxyModel::index(int row, int column, const QModelIndex&) const
{
    return this->createIndex(row, column, nullptr);
}

std::optional<QModelIndex> ProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();

    const int sourceRow = proxyIndex.row() * m_columnCount + proxyIndex.column();
    if (sourceRow < sourceModel()->rowCount())
        return this->sourceModel()->index(sourceRow, 0);

    return {};
}

QModelIndex ProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();

    const int proxyRow = sourceIndex.row() / m_columnCount;
    const int proxyColumn = sourceIndex.row() % m_columnCount;
    return this->index(proxyRow, proxyColumn, QModelIndex());
}

void ProxyModel::onDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    emit this->dataChanged(this->mapFromSource(topLeft), this->mapFromSource(bottomRight), roles);
}

View::View(QWidget* parent)
    : QTableView(parent)
{
    this->horizontalHeader()->hide();
    this->verticalHeader()->hide();
    this->horizontalHeader()->setDefaultSectionSize(m_itemSize.width());
    this->verticalHeader()->setDefaultSectionSize(m_itemSize.height());

    this->setFrameShape(QFrame::NoFrame);
    this->setGridStyle(Qt::NoPen);

    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    this->setMouseTracking(true); // To enable hover
}

void View::setItemSize(QSize size)
{
    this->horizontalHeader()->setDefaultSectionSize(size.width());
    this->verticalHeader()->setDefaultSectionSize(size.height());
    m_itemSize = size;
}

void View::leaveEvent(QEvent*)
{
    QHoverEvent hoverEvent(QEvent::HoverLeave, {}, {});
    this->viewportEvent(&hoverEvent); // Seemingly needed to kill the hover paint
}

} // namespace GridHelper
} // namespace Mayo
