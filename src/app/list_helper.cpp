/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "list_helper.h"

#include "qtgui_utils.h"
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>

#if 0
QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter* p, QImage& blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

namespace QtGuiUtils {

static QPixmap blurredPixmap(const QPixmap& pixmap, int radius)
{
    if (pixmap.isNull())
        return pixmap;

    QImage img = pixmap.toImage();
    QPixmap pmDst(img.size());
    pmDst.fill(Qt::transparent);
    QPainter painter(&pmDst);
    qt_blurImage(&painter, img, radius, true, false);
    return pmDst;
}

} // namespace QtGuiUtils
#endif

namespace Mayo::ListHelper {

static QtGuiUtils::FontChange fontChange(const QWidget* widget) {
    return QtGuiUtils::FontChange(widget->font());
}

Model::Model(QObject* parent)
    : QAbstractListModel(parent)
{
    this->setStorage(std::make_unique<DefaultModelStorage<ModelItem>>());
}

int Model::rowCount(const QModelIndex&) const
{
    return this->hasStorage() ? this->storage()->count() : 0;
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!this->hasStorage())
        return QVariant();

    if (!index.isValid() || index.row() >= this->storage()->count())
        return QVariant();

    const ModelItem* item = this->storage()->at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return item->name;
    case RoleItemPtr:
        return QVariant::fromValue(const_cast<ModelItem*>(item));
    case RoleItemImage: {
        QPixmap pixmap;
        if (QPixmapCache::find(item->imageUrl, &pixmap))
            return pixmap;
        else
            return this->findPixmap(item->imageUrl);
    }
    default:
        return QVariant();
    }
}

const ModelItem* Model::itemPtrAt(const QModelIndex& index)
{
    return index.data(RoleItemPtr).value<ModelItem*>();
}

QPixmap Model::itemPixmapAt(const QModelIndex& index)
{
    return index.data(RoleItemImage).value<QPixmap>();
}

void Model::setStorage(std::unique_ptr<ModelStorage> ptr)
{
    m_storage = std::move(ptr);
}

ItemDelegate::ItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent),
      m_frameColor(220, 220, 220),
      m_pixmapColor(qApp->palette().color(QPalette::Button)),
      m_textColor(qApp->palette().color(QPalette::WindowText))
{
    m_itemAnimation.setDuration(250);
    m_itemAnimation.setEasingCurve(QEasingCurve::OutQuad);
    m_itemAnimation.setLoopCount(1);
    m_itemAnimation.setStartValue(0);
    m_itemAnimation.setEndValue(m_itemSize.height());
    QObject::connect(
        &m_itemAnimation, &QVariantAnimation::valueChanged,
        this, &ItemDelegate::drawItem
    );
}

void ItemDelegate::paint(
        QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index
    ) const
{
    const ModelItem* item = Model::itemPtrAt(index);
    if (!item)
        return;

    const QRect area = option.rect;
    const int itemBottom = m_itemSize.height() - m_itemSpacing;
    const int d = m_itemSpacing;
    const int x = area.x() + d;
    const int y = area.y() + d;
    const int w = area.width() - 2 * d - m_itemSpacing;
    const int h = area.height() - 2 * d;
    const bool hovered = option.state & QStyle::State_MouseOver;
    const int yShift = itemBottom - 20;
    const int yName = itemBottom - 20;
    const QRect rectText = QRect(x, y + yName, w, h);
    const QPixmap pm = Model::itemPixmapAt(index);

    QTextOption textOption;
    textOption.setWrapMode(item->textWrapMode);
    int offset = 0;
    if (hovered) {
        if (index != m_previousIndex) {
            m_previousIndex = index;
            m_area = area;
            m_widget = qobject_cast<QAbstractItemView*>(const_cast<QWidget*>(option.widget));
            m_itemAnimation.start(QAbstractAnimation::KeepWhenStopped);
            //m_blurredPixmap = QtGuiUtils::blurredPixmap(pm, 20);
        }

        offset = m_itemAnimation.currentValue().toInt();
        if (offset > yShift) {
            offset = yShift;
            m_itemAnimation.stop();
        }
    }

    const QFontMetrics fm(option.widget->font());
    const QRect rectShiftedText = rectText.adjusted(0, -offset, 0, -offset);

    // Draw pixmap
    if (offset == 0) {
        const QRect inner({ x + 11, y - offset }, m_itemPixmapSize);
        QRect rectPixmap = inner;
        if (!pm.isNull()) {
            QPoint posPixmap = rectPixmap.center();
            posPixmap.rx() -= pm.width() / pm.devicePixelRatio() / 2;
            posPixmap.ry() -= pm.height() / pm.devicePixelRatio() / 2;
            painter->drawPixmap(posPixmap, pm);
        }
        else { // Description text as fallback
            painter->setPen(m_textColor);
            painter->setFont(fontChange(option.widget).adjustSize(-1));
            painter->drawText(
                        rectPixmap.adjusted(6, m_itemSpacing, -6, -m_itemSpacing),
                        item->description,
                        textOption);
        }

        painter->setPen(m_pixmapColor);
        painter->drawRect(rectPixmap.adjusted(-1, -1, -1, -1));
    }
    else {
        if (!m_blurredPixmap.isNull()) {
            const QRect inner({ x + 11, y }, m_itemPixmapSize);
            QPoint posPixmap = inner.center();
            posPixmap.rx() -= pm.width() / pm.devicePixelRatio() / 2;
            posPixmap.ry() -= pm.height() / pm.devicePixelRatio() / 2;
            painter->drawPixmap(posPixmap, m_blurredPixmap);
        }
    }

    // Draw title
    painter->setPen(m_textColor);
    QRectF rectName;
    if (offset) {
        painter->setFont(fontChange(option.widget).adjustSize(1).bold(true));
        rectName = painter->boundingRect(rectShiftedText, item->name, textOption);
        painter->drawText(rectName, item->name, textOption);
    }
    else {
        painter->setFont(fontChange(option.widget).adjustSize(1));
        rectName = QRect(x + 11, y + yName, x + w, y + yName + 20);
        const QString elidedName = fm.elidedText(item->name, Qt::ElideRight, w - 20);
        painter->drawText(rectName, elidedName);
    }

    // Draw separator line below title
    if (offset) {
        const int ll = rectName.bottom() + 5;
        painter->setPen(m_frameColor);
        painter->drawLine(area.x(), ll, area.x() + area.width(), ll);
    }

    // Draw description
    if (offset) {
        const int dd = rectName.height() + m_itemSpacing;
        const QRect rectDescription = rectShiftedText.adjusted(0, dd, 0, dd);
        painter->setPen(m_textColor);
        painter->setFont(fontChange(option.widget).adjustSize(-1));
        painter->drawText(rectDescription, item->description, textOption);
    }

    // Draw box when item is hovered
    if (hovered) {
        painter->setPen(m_frameColor);
        painter->drawRect(area.adjusted(0, 0, -1, -1));
    }
}

void ItemDelegate::setItemSize(QSize size)
{
    m_itemSize = size;
    m_itemAnimation.setEndValue(size.height());
}

bool ItemDelegate::editorEvent(
        QEvent* event,
        QAbstractItemModel* model,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    )
{
    if (event->type() == QEvent::MouseButtonRelease) {
        const ModelItem* item = Model::itemPtrAt(index);
        if (!item)
            return false;

        auto mev = static_cast<const QMouseEvent*>(event);
        if (mev->button() != Qt::LeftButton)
            return false;

        if (index.isValid())
            this->clickAction(item);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ItemDelegate::drawItem()
{
    if (m_widget)
        m_widget->viewport()->update(m_area);
}

} // namespace Mayo::ListHelper
