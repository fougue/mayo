/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtWidgets/QTableView>
#include <optional>

// Provides helper tools for "grid" view
namespace Mayo::GridHelper {

// Provides a proxy model to layout items along a grid(this setColumnCount())
class ProxyModel : public QAbstractItemModel {
public:
    void setSourceModel(QAbstractItemModel* newModel);
    QAbstractItemModel* sourceModel() const { return m_sourceModel; }

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool hasChildren(const QModelIndex& parent) const override;
    QModelIndex index(int row, int column, const QModelIndex&) const override;
    QModelIndex parent(const QModelIndex&) const override { return QModelIndex(); }
    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;
    void setColumnCount(int count);

    std::optional<QModelIndex> mapToSource(const QModelIndex& proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;

private:
    void onDataChanged(
        const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles
    );

    QAbstractItemModel* m_sourceModel = nullptr;
    int m_columnCount = 1;
};

// Provides Qt view with grid layout
class View : public QTableView {
public:
    View(QWidget* parent);

    QSize itemSize() const { return m_itemSize; }
    void setItemSize(QSize size);

protected:
    void leaveEvent(QEvent*) override;

private:
    QSize m_itemSize = { 230, 180 };
};

} // namespace Mayo::GridHelper
