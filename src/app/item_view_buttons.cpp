/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "item_view_buttons.h"

#include "proxy_styled_item_delegate.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QStyledItemDelegate>
#include <unordered_map>

namespace Mayo {

namespace {

template<typename CallValueType, typename ValueType, typename ClassType>
void checkedAssign(ValueType ClassType::*attrMember, ClassType* object, CallValueType value)
{
    if (object && attrMember)
        object->*attrMember = value;
}

} // namespace

class ItemViewButtons::Private
{
public:
    class ProxyItemDelegate : public ProxyStyledItemDelegate {
    public:
        ProxyItemDelegate(
            const ItemViewButtons* itemBtns, QStyledItemDelegate* srcDelegate, QObject* parent = nullptr
        );

        void paint(
            QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index
        ) const override;

    private:
        const ItemViewButtons* m_itemBtns;
    };

    struct ButtonInfo {
        int index;
        QIcon icon;
        QSize iconSize;
        QString toolTip;
        int matchRole;
        QVariant matchData;
        int displayColumn;
        ItemViewButtons::ItemSide itemSide;
        ItemViewButtons::DisplayModes itemDisplayModes;
    };

    Private(ItemViewButtons* backPtr);

    const ButtonInfo* buttonInfo(int btnId) const;
    ButtonInfo* mutableButtonInfo(int btnId);
    void setAllIsOverButtonState(bool on);
    QModelIndex modelIndexForButtonDisplay(const QModelIndex& index) const;
    void itemViewUpdateAt(const QModelIndex& index);
    void paintButton(ButtonInfo* btnInfo, QPainter* painter, const QStyleOptionViewItem& option);
    void resetButtonUnderMouseState();

    QAbstractItemView* m_view = nullptr;
    std::unordered_map<int, ButtonInfo> m_btnInfos;
    const ButtonInfo* m_buttonUnderMouse = nullptr;

private:
    ItemViewButtons* m_backPtr = nullptr;
};

ItemViewButtons::Private::ProxyItemDelegate::ProxyItemDelegate(
        const ItemViewButtons* itemBtns,
        QStyledItemDelegate* srcDelegate,
        QObject* parent
    )
    : ProxyStyledItemDelegate(srcDelegate, parent),
      m_itemBtns(itemBtns)
{
}

void ItemViewButtons::Private::ProxyItemDelegate::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const
{
    ProxyStyledItemDelegate::paint(painter, option, index);
    if (m_itemBtns)
        m_itemBtns->paint(painter, option, index);
}

ItemViewButtons::Private::Private(ItemViewButtons* backPtr)
    : m_backPtr(backPtr)
{
}

const ItemViewButtons::Private::ButtonInfo* ItemViewButtons::Private::buttonInfo(int btnId) const
{
    const auto iBtnInfo = m_btnInfos.find(btnId);
    return iBtnInfo != m_btnInfos.cend() ? &(iBtnInfo->second) : nullptr;
}

ItemViewButtons::Private::ButtonInfo* ItemViewButtons::Private::mutableButtonInfo(int btnId)
{
    auto iBtnInfo = m_btnInfos.find(btnId);
    return iBtnInfo != m_btnInfos.end() ? &(iBtnInfo->second) : nullptr;
}

QModelIndex ItemViewButtons::Private::modelIndexForButtonDisplay(const QModelIndex& index) const
{
    const int btnIndex = m_backPtr->buttonAtModelIndex(index);
    const auto btnInfo = this->buttonInfo(btnIndex);
    if (btnInfo && btnInfo->displayColumn != -1)
        return index.sibling(index.row(), btnInfo->displayColumn);
    else
        return index;
}

void ItemViewButtons::Private::itemViewUpdateAt(const QModelIndex& index)
{
    const QModelIndex displayIndex = this->modelIndexForButtonDisplay(index);
    if (index.isValid())
        m_view->update(index);

    if (displayIndex != index && displayIndex.isValid())
        m_view->update(displayIndex);
}

void ItemViewButtons::Private::paintButton(
        ButtonInfo* btnInfo,
        QPainter* painter,
        const QStyleOptionViewItem& option
    )
{
    if (!btnInfo || !painter)
        return;

    const bool isValidBtnIconSize = btnInfo->iconSize.isValid();
    const int pixWidth = isValidBtnIconSize ? btnInfo->iconSize.width() : option.rect.height();
    const int pixHeight = isValidBtnIconSize ? btnInfo->iconSize.height() : option.rect.height();

    QRect pixRect;
    const int yPixPos = option.rect.top() + (option.rect.height() - pixHeight) / 2;
    if (btnInfo->itemSide == ItemViewButtons::ItemLeftSide)
        pixRect = QRect(option.rect.left() + 2, yPixPos, pixWidth, pixHeight);
    else
        pixRect = QRect(option.rect.right() - pixWidth - 2, yPixPos, pixWidth, pixHeight);

    const bool isInsideButtonRegion = pixRect.contains(m_view->viewport()->mapFromGlobal(QCursor::pos()));
    const QIcon icon = btnInfo->icon;
    const QPixmap pix = icon.pixmap(pixWidth, pixHeight, isInsideButtonRegion ? QIcon::Active : QIcon::Normal);
    painter->drawPixmap(pixRect, pix);

    if (isInsideButtonRegion)
        m_buttonUnderMouse = btnInfo;
}

void ItemViewButtons::Private::resetButtonUnderMouseState()
{
    m_buttonUnderMouse = nullptr;
}

/*! \fn void ItemViewButtons::buttonClicked(int btnId, const QModelIndex& index)
 *  \brief This signal is emitted when a button previously added with addButton()
 *         is clicked (i.e. pressed down then released while the mouse cursor is
 *         inside the button)
 *
 *  \param btnId Identifier of the button clicked (this is the id that was
 *               passed to addButton())
 *  \param index Index of the item model where the button click occurred
 */

ItemViewButtons::ItemViewButtons(QAbstractItemView* view, QObject* parent)
    : QObject(parent),
      d(new Private(this))
{
    d->m_view = view;
    if (view) {
        view->viewport()->setMouseTracking(true);
        view->viewport()->installEventFilter(this);
    }
}

ItemViewButtons::~ItemViewButtons()
{
    delete d;
}

QAbstractItemView* ItemViewButtons::itemView() const
{
    return d->m_view;
}

void ItemViewButtons::reset()
{
    d->resetButtonUnderMouseState();
}

bool ItemViewButtons::eventFilter(QObject* object, QEvent* event)
{
    if (object == this->itemView()->viewport()) {
        // If mouse event, retrieve item's model index under mouse
        const QMouseEvent* mouseEvent = nullptr;
        if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonRelease)
            mouseEvent = static_cast<const QMouseEvent*>(event);

        const QModelIndex modelIndexUnderMouse =
                mouseEvent ? this->itemView()->indexAt(mouseEvent->pos()) : QModelIndex();

        // Process input event
        switch (event->type()) {
        case QEvent::Leave:
        case QEvent::MouseMove: {
            d->resetButtonUnderMouseState();
            d->itemViewUpdateAt(modelIndexUnderMouse);
            return true;
        }
        case QEvent::MouseButtonRelease: {
            if (mouseEvent && mouseEvent->button() == Qt::LeftButton && d->m_buttonUnderMouse) {
                emit buttonClicked(d->m_buttonUnderMouse->index, modelIndexUnderMouse);
                return true;
            }

            return false;
        }
        case QEvent::ToolTip: {
            const QString toolTip = d->m_buttonUnderMouse ? d->m_buttonUnderMouse->toolTip : QString();
            if (!toolTip.isEmpty()) {
                QToolTip::showText(QCursor::pos(), toolTip, this->itemView());
                return true;
            }

            return false;
        }
        default:
            break;
        }
    }

    return false;
}

void ItemViewButtons::paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index
    ) const
{
    bool mouseIsOver = false;
    if (painter && painter->device() && painter->device()->devType() == QInternal::Widget) {
        auto w = static_cast<QWidget*>(painter->device());
        if (w) {
            QPoint mousePos = QCursor::pos();
            QPoint wMousePos = w->mapFromGlobal(mousePos);
            mouseIsOver = option.rect.contains(wMousePos);
        }
    }

    //  QStyledItemDelegate::paint(painter, option, index);

    if (this->itemView()->isEnabled()) {
        QStyleOptionViewItem optionForBtn(option);
        optionForBtn.rect = this->itemView()->visualRect(d->modelIndexForButtonDisplay(index));
        const int btnIndex = this->buttonAtModelIndex(index);
        Private::ButtonInfo* btnInfo = d->mutableButtonInfo(btnIndex);
        if (!btnInfo)
            return;

        if (btnInfo->itemDisplayModes.testFlag(DisplayPermanent)) {
            d->paintButton(btnInfo, painter, optionForBtn);
        }
        else if (btnInfo->itemDisplayModes.testFlag(DisplayOnDetection)) {
            if (mouseIsOver)
                d->paintButton(btnInfo, painter, optionForBtn);
            else
                painter->fillRect(optionForBtn.rect, optionForBtn.backgroundBrush);
        }
        else if (btnInfo->itemDisplayModes.testFlag(DisplayWhenItemSelected)) {
            if (option.state.testFlag(QStyle::State_Selected))
                d->paintButton(btnInfo, painter, optionForBtn);
            else
                painter->fillRect(optionForBtn.rect, optionForBtn.backgroundBrush);
        }
    }
    else {
        d->resetButtonUnderMouseState();
        //    d->setAllIsOverButtonState(false);
    }
}

/*!
 * \brief Add a button to be furthered configured with setButtonXxx() functions
 * \param btnId Index of the button (used later to reference the button)
 * \param icon Icon of the button (ItemViewButtons supports QIcon::Active which
 *             can be used to display an highlighted pixmap when the mouse is
 *             hovering the button)
 * \param toolTip Tool-tip to be displayed when the mouse stays over the button
 *
 *  Does nothing if button index \p btnId is already used by some other button.
 */
void ItemViewButtons::addButton(int btnId, const QIcon& icon, const QString& toolTip)
{
    if (d->m_btnInfos.find(btnId) == d->m_btnInfos.cend()) {
        Private::ButtonInfo info;
        info.index = btnId;
        info.icon = icon;
        info.toolTip = toolTip;
        info.matchRole = Qt::UserRole + 1;
        info.displayColumn = -1;
        info.itemSide = ItemRightSide;
        info.itemDisplayModes = DisplayOnDetection;
        d->m_btnInfos.insert({ btnId, info });
    }
    else {
        qWarning() << QString("%1 : there is already a button of index '%2'")
                      .arg(Q_FUNC_INFO).arg(btnId);
    }
}

/*!
 * \brief Copy all properties of a button into another
 * \param srcBtnId Index of the source button
 * \param dstBtnId Index of the destination button
 */
void ItemViewButtons::copyButtonProperties(int srcBtnId, int dstBtnId)
{
    if (srcBtnId == dstBtnId)
        return;

    const Private::ButtonInfo* srcBtnInfo = d->buttonInfo(srcBtnId);
    Private::ButtonInfo* dstBtnInfo = d->mutableButtonInfo(dstBtnId);

    if (srcBtnInfo) {
        if (dstBtnInfo) {
            *dstBtnInfo = *srcBtnInfo;
            dstBtnInfo->index = dstBtnId; // Restore destination button index
        }
        else {
            qWarning() << QString("%1 : no destination button of index '%1'")
                          .arg(Q_FUNC_INFO).arg(dstBtnId);
        }
    }
    else {
        qWarning() << QString("%1 : no source button of index '%1'")
                      .arg(Q_FUNC_INFO).arg(srcBtnId);
    }
}

/*!
 * \brief The role used when matching item data for button detection
 *
 * \param btnId Index of the button
 * \returns -1 If button does not exist or if no matching role was set
 *
 * \sa buttonDetectionMatchData()
 * \sa QModelIndex::data()
 * \sa Qt::ItemDataRole
 */
int ItemViewButtons::buttonDetectionMatchRole(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->matchRole : -1;
}

/*!
 * \brief The data to be matched for button detection
 *
 * \param btnId Index of the button
 * \returns QVariant() If button does not exist
 *
 * \sa buttonDetectionMatchRole()
 */
QVariant ItemViewButtons::buttonDetectionMatchData(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->matchData : QVariant();
}

/*!
 * \brief ItemViewButtons::setButtonDetection
 *
 * \param btnId Index of the button
 * \param matchRole The role used when matching item data for button detection.
 *        In case the button has to be displayed no matter the item, then set
 *        \p matchRole to -1
 * \param matchData The data to be matched for button detection
 *
 * \sa buttonDetectionMatchData()
 * \sa buttonDetectionMatchRole()
 * \sa QModelIndex::data()
 * \sa Qt::ItemDataRole
 */
void ItemViewButtons::setButtonDetection(int btnId, int matchRole, const QVariant& matchData)
{
    Private::ButtonInfo* btnInfo = d->mutableButtonInfo(btnId);
    if (btnInfo) {
        btnInfo->matchRole = matchRole;
        btnInfo->matchData = matchData;
    }
}

/*!
 * \brief Index of the view column where the button is displayed (when detected)
 * \param btnId Index of the button
 * \returns -1 If button does not exist
 */
int ItemViewButtons::buttonDisplayColumn(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->displayColumn : -1;
}

void ItemViewButtons::setButtonDisplayColumn(int btnId, int col)
{
    checkedAssign(&Private::ButtonInfo::displayColumn, d->mutableButtonInfo(btnId), col);
}

/*!
 * \brief Side in the item's cell where the button is displayed
 * \param btnId Index of the button
 * \returns -1 If button does not exist
 */
int ItemViewButtons::buttonItemSide(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->itemSide : -1;
}

void ItemViewButtons::setButtonItemSide(int btnId, ItemSide side)
{
    checkedAssign(&Private::ButtonInfo::itemSide, d->mutableButtonInfo(btnId), side);
}

ItemViewButtons::DisplayModes ItemViewButtons::buttonDisplayModes(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->itemDisplayModes : DisplayModes();
}

void ItemViewButtons::setButtonDisplayModes(int btnId, DisplayModes modes)
{
    checkedAssign(&Private::ButtonInfo::itemDisplayModes, d->mutableButtonInfo(btnId), modes);
}

QIcon ItemViewButtons::buttonIcon(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->icon : QIcon();
}

void ItemViewButtons::setButtonIcon(int btnId, const QIcon& icon)
{
    checkedAssign(&Private::ButtonInfo::icon, d->mutableButtonInfo(btnId), icon);
}

QSize ItemViewButtons::buttonIconSize(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->iconSize : QSize();
}

void ItemViewButtons::setButtonIconSize(int btnId, const QSize& size)
{
    checkedAssign(&Private::ButtonInfo::iconSize, d->mutableButtonInfo(btnId), size);
}

QString ItemViewButtons::buttonToolTip(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo ? btnInfo->toolTip : QString();
}

void ItemViewButtons::setButtonToolTip(int btnId, const QString& toolTip)
{
    checkedAssign(&Private::ButtonInfo::toolTip, d->mutableButtonInfo(btnId), toolTip);
}

//! Install a delegate for the attached view item, allowing the button mechanism to work
void ItemViewButtons::installDefaultItemDelegate()
{
    if (d->m_view)
        d->m_view->setItemDelegate(this->createProxyItemDelegate(nullptr));
}

/*!
 * Create a proxy delegate around \p sourceDelegate to be further installed with
 * QAbstractItemView::setItemDelegate()
 *
 *  This is useful when you have a delegate for an item view but for some reason
 *  don't want to modify it to integrate with ItemViewButtons
 */
QStyledItemDelegate* ItemViewButtons::createProxyItemDelegate(
        QStyledItemDelegate* sourceDelegate, QObject* parent
    ) const
{
    return new Private::ProxyItemDelegate(this, sourceDelegate, parent);
}

int ItemViewButtons::buttonAtModelIndex(const QModelIndex& index) const
{
    for (const auto& [id, btnInfo] : d->m_btnInfos) {
        if (btnInfo.matchRole < 0)
            return id;

        const QVariant modelItemData = index.data(btnInfo.matchRole);
        if ((!btnInfo.matchData.isNull() && btnInfo.matchData.isValid())
                && (!modelItemData.isNull() && modelItemData.isValid())
                && (btnInfo.matchData == modelItemData))
        {
            return id;
        }
    }

    return -1;
}

} // namespace Mayo
