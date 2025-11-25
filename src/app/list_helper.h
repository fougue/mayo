/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QMetaType>
#include <QtCore/QVariantAnimation>
#include <QtGui/QTextOption>
#include <QtWidgets/QStyledItemDelegate>
#include <memory>
#include <type_traits>
#include <vector>

namespace Mayo::ListHelper {

struct ModelItem {
    QString name;
    QString description;
    QString imageUrl;
    QTextOption::WrapMode textWrapMode = QTextOption::NoWrap;
};

struct ModelStorage {
    virtual ~ModelStorage() {}
    virtual int count() const = 0;
    virtual const ModelItem* at(int i) const = 0;
};

template<typename ItemType> struct DefaultModelStorage : public ModelStorage {
    static_assert(!std::is_pointer<ItemType>::value);
    static_assert(std::is_base_of<ModelItem, ItemType>::value);
    int count() const override { return int(m_items.size()); }
    const ItemType* at(int i) const override { return &m_items.at(i); }
    std::vector<ItemType> m_items;
};

class Model : public QAbstractListModel {
public:
    enum DataRole {
        RoleItemPtr = Qt::UserRole + 1,
        RoleItemImage
    };

    Model(QObject* parent);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QPixmap findPixmap(const QString& url) const = 0;

    static const ModelItem* itemPtrAt(const QModelIndex& index);
    static QPixmap itemPixmapAt(const QModelIndex& index);

protected:
    void setStorage(std::unique_ptr<ModelStorage> ptr); // Should be called in constructor

private:
    bool hasStorage() const { return this->storage() != nullptr; }
    const ModelStorage* storage() const { return m_storage.get(); }
    std::unique_ptr<ModelStorage> m_storage;
};

class ItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ItemDelegate(QObject* parent = nullptr);

    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index
        ) const override;

    QSize itemSize() const { return m_itemSize; }
    void setItemSize(QSize size);

    int itemSpacing() const { return m_itemSpacing; }
    void setItemSpacing(int spacing) { m_itemSpacing = spacing; }

    QSize itemPixmapSize() const { return m_itemPixmapSize; }
    void setItemPixmapSize(QSize size) { m_itemPixmapSize = size; }

protected:
    bool editorEvent(
            QEvent* event,
            QAbstractItemModel* model,
            const QStyleOptionViewItem& option,
            const QModelIndex& index
        ) override;

    virtual void clickAction(const ModelItem*) const {
    }

private:
    void drawItem();

    QColor m_frameColor;
    QColor m_pixmapColor;
    QColor m_textColor;
    QSize m_itemSize = { 230, 180 };
    int m_itemSpacing = 10;
    QSize m_itemPixmapSize = { 190, 150 };

    mutable QModelIndex m_previousIndex;
    mutable QVariantAnimation m_itemAnimation;
    mutable QRect m_area;
    mutable QAbstractItemView* m_widget = nullptr;
    mutable QPixmap m_blurredPixmap;
};

} // namespace Mayo::ListHelper

Q_DECLARE_METATYPE(Mayo::ListHelper::ModelItem*)
