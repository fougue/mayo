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

#include "item_view_buttons.h"

#include "proxy_styled_item_delegate.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
// QtWidgets
#include <QAbstractItemView>
#include <QToolTip>
#include <QStyledItemDelegate>

namespace qtgui {

namespace cpp {

template<typename CALL_VALUE_TYPE, typename VALUE_TYPE, typename CLASS>
void checkedAssign(VALUE_TYPE CLASS::*attrMember, CLASS* object, CALL_VALUE_TYPE value)
{
    if (object != nullptr && attrMember != nullptr)
        object->*attrMember = value;
}

} // namespace cpp

/*! \class ItemViewButtonsPrivate
 *  \brief Internal (pimpl of ItemViewButtons)
 */

class ItemViewButtons::Private
{
public:
    class ProxyItemDelegate : public ProxyStyledItemDelegate
    {
    public:
        ProxyItemDelegate(
                const ItemViewButtons* itemBtns,
                QStyledItemDelegate* srcDelegate,
                QObject* parent = nullptr);

        void paint(
                QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    private:
        const ItemViewButtons* m_itemBtns;
    };

    struct ButtonInfo
    {
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
    void paintButton(
            ButtonInfo *btnInfo,
            QPainter* painter,
            const QStyleOptionViewItem& option);
    void resetButtonUnderMouseState();

    QAbstractItemView* m_view;
    QHash<int, ButtonInfo> m_btnInfos;
    QModelIndex m_prevModelIndexUnderMouse;
    const ButtonInfo* m_buttonUnderMouse;

private:
    ItemViewButtons* m_backPtr;
};

ItemViewButtons::Private::ProxyItemDelegate::ProxyItemDelegate(
        const ItemViewButtons *itemBtns,
        QStyledItemDelegate *srcDelegate,
        QObject* parent)
    : ProxyStyledItemDelegate(srcDelegate, parent),
      m_itemBtns(itemBtns)
{
}

void ItemViewButtons::Private::ProxyItemDelegate::paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    ProxyStyledItemDelegate::paint(painter, option, index);
    if (m_itemBtns != nullptr)
        m_itemBtns->paint(painter, option, index);
}

ItemViewButtons::Private::Private(ItemViewButtons *backPtr)
    : m_view(nullptr),
      m_buttonUnderMouse(nullptr),
      m_backPtr(backPtr)
{
}

const ItemViewButtons::Private::ButtonInfo*
ItemViewButtons::Private::buttonInfo(int btnId) const
{
    const auto iBtnInfo = m_btnInfos.find(btnId);
    if (iBtnInfo != m_btnInfos.constEnd())
        return &(iBtnInfo.value());
    return nullptr;
}

ItemViewButtons::Private::ButtonInfo*
ItemViewButtons::Private::mutableButtonInfo(int btnId)
{
    auto iBtnInfo = m_btnInfos.find(btnId);
    if (iBtnInfo != m_btnInfos.end())
        return &(iBtnInfo.value());
    return nullptr;
}

QModelIndex ItemViewButtons::Private::modelIndexForButtonDisplay(
        const QModelIndex &index) const
{
    const int btnIndex = m_backPtr->buttonAtModelIndex(index);
    const auto btnInfo = this->buttonInfo(btnIndex);
    if (btnInfo != nullptr && btnInfo->displayColumn != -1)
        return index.sibling(index.row(), btnInfo->displayColumn);
    return index;
}

void ItemViewButtons::Private::itemViewUpdateAt(const QModelIndex &index)
{
    const QModelIndex displayIndex = this->modelIndexForButtonDisplay(index);
    if (index.isValid())
        m_view->update(index);
    if (displayIndex != index && displayIndex.isValid())
        m_view->update(displayIndex);
}

void ItemViewButtons::Private::paintButton(
        ButtonInfo *btnInfo,
        QPainter *painter,
        const QStyleOptionViewItem &option)
{
    if (btnInfo == nullptr || painter == nullptr)
        return;

    const bool isValidBtnIconSize = btnInfo->iconSize.isValid();
    const int pixWidth =
            isValidBtnIconSize ?
                btnInfo->iconSize.width() :
                option.rect.height();
    const int pixHeight =
            isValidBtnIconSize ?
                btnInfo->iconSize.height() :
                option.rect.height();

    QRect pixRect;
    const int yPixPos =
            option.rect.top() + (option.rect.height() - pixHeight) / 2;
    if (btnInfo->itemSide == ItemViewButtons::ItemLeftSide)
        pixRect = QRect(option.rect.left() + 2, yPixPos, pixWidth, pixHeight);
    else
        pixRect = QRect(option.rect.right() - pixWidth - 2, yPixPos, pixWidth, pixHeight);

    const bool isInsideButtonRegion =
            pixRect.contains(m_view->viewport()->mapFromGlobal(QCursor::pos()));
    const QIcon icon = btnInfo->icon;
    const QPixmap pix =
            icon.pixmap(
                pixWidth,
                pixHeight,
                isInsideButtonRegion ? QIcon::Active : QIcon::Normal);
    painter->drawPixmap(pixRect, pix);

    if (isInsideButtonRegion)
        m_buttonUnderMouse = btnInfo;
}

void ItemViewButtons::Private::resetButtonUnderMouseState()
{
    m_buttonUnderMouse = nullptr;
}

/*!
 * \class ItemViewButtons
 * \brief Provides buttons integrated to items displayed by QAbstractItemView
 *
 * qtgui::ItemViewButtons allows to add buttons inside any QAbstractItemView
 * without subclassing the item-view class.
 *
 * It only requires that its paint() method is called whenever any view item has
 * to be drawn. If you have a custom delegate (eg. a subclass of QStyledItemDelegate)
 * then just call at some point qtgui::ItemViewButtons::paint() inside
 * the delegate's paint() method :
 * \code
 * void MyCustomDeleagate::paint(QPainter* painter,
 *                               const QStyleOptionViewItem& option,
 *                               const QModelIndex& index) const
 * {
 *   QStyledItemDelegate::paint(painter, option, index);
 *   // Specific paint operations ...
 *
 *   m_itemViewBtns->paint(painter, option, index);
 * }
 * \endcode
 *
 * If you do not want to modify your delegate class then createProxyItemDelegate()
 * might be the right option : this will create a new delegate around yours with
 * the paint() method correctly called.
 *
 * If the item-view does not use any delegate then just call installDefaultItemDelegate()
 *
 * ItemViewButtons notifies any button click with signal buttonClicked()
 *
 * \example qttools/item_view_buttons/main.cpp
 *
 * \headerfile item_view_buttons.h <qttools/gui/item_view_buttons.h>
 * \ingroup qttools_gui
 */

/*! \fn void ItemViewButtons::buttonClicked(int btnId, const QModelIndex& index)
 *  \brief This signal is emitted when a button previously added with addButton()
 *         is clicked (i.e. pressed down then released while the mouse cursor is
 *         inside the button)
 *
 *  \param btnId Identifier of the button clicked (this is the id that was
 *               passed to addButton())
 *  \param index Index of the item model where the button click occured
 */

ItemViewButtons::ItemViewButtons(QAbstractItemView* view, QObject *parent)
    : QObject(parent),
      d(new Private(this))
{
    d->m_view = view;
    if (view != nullptr) {
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
    d->m_prevModelIndexUnderMouse = QModelIndex();
    d->resetButtonUnderMouseState();
}

bool ItemViewButtons::eventFilter(QObject *object, QEvent *event)
{
    if (object == this->itemView()->viewport()) {
        // If mouse event, retrieve item's model index under mouse
        const QMouseEvent* mouseEvent = nullptr;
        if (event->type() == QEvent::MouseMove
                || event->type() == QEvent::MouseButtonRelease)
        {
            mouseEvent = static_cast<const QMouseEvent*>(event);
        }
        const QModelIndex modelIndexUnderMouse =
                mouseEvent != nullptr ?
                    this->itemView()->indexAt(mouseEvent->pos()) :
                    QModelIndex();

        // Process input event
        switch (event->type()) {
        case QEvent::Leave:
        case QEvent::MouseMove: {
            d->resetButtonUnderMouseState();
            if (d->m_prevModelIndexUnderMouse != modelIndexUnderMouse)
                d->itemViewUpdateAt(d->m_prevModelIndexUnderMouse);
            d->itemViewUpdateAt(modelIndexUnderMouse);
            d->m_prevModelIndexUnderMouse = modelIndexUnderMouse;
            return true;
        }
        case QEvent::MouseButtonRelease: {
            if (mouseEvent != nullptr
                    && mouseEvent->button() == Qt::LeftButton
                    && d->m_buttonUnderMouse != nullptr)
            {
                emit buttonClicked(
                            d->m_buttonUnderMouse->index, modelIndexUnderMouse);
                return true;
            }
            return false;
        }
        case QEvent::ToolTip: {
            const QString toolTip =
                    d->m_buttonUnderMouse != nullptr ?
                        d->m_buttonUnderMouse->toolTip :
                        QString();
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
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    bool mouseIsOver = false;
    if (painter != nullptr
            && painter->device() != nullptr
            && painter->device()->devType() == QInternal::Widget)
    {
        QWidget* w = static_cast<QWidget*>(painter->device());
        if (w != nullptr) {
            QPoint mousePos = QCursor::pos();
            QPoint wMousePos = w->mapFromGlobal(mousePos);
            mouseIsOver = option.rect.contains(wMousePos);
        }
    }

    //  QStyledItemDelegate::paint(painter, option, index);

    if (this->itemView()->isEnabled()) {
        QStyleOptionViewItemV4 optionForBtn(option);
        optionForBtn.rect =
                this->itemView()->visualRect(d->modelIndexForButtonDisplay(index));
        const int btnIndex = this->buttonAtModelIndex(index);
        Private::ButtonInfo* btnInfo = d->mutableButtonInfo(btnIndex);
        if (btnInfo == nullptr)
            return;

        // Check if button can be displayed
        if (btnInfo->itemDisplayModes.testFlag(DisplayWhenItemSelected)
                && !option.state.testFlag(QStyle::State_Selected))
        {
            //      painter->fillRect(optionForBtn.rect, optionForBtn.backgroundBrush);
            return;
        }

        if (btnInfo->itemDisplayModes.testFlag(DisplayPermanent)) {
            d->paintButton(btnInfo, painter, optionForBtn);
        }
        else if (btnInfo->itemDisplayModes.testFlag(DisplayOnDetection)) {
            if (mouseIsOver)
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
void ItemViewButtons::addButton(
        int btnId, const QIcon &icon, const QString &toolTip)
{
    if (!d->m_btnInfos.contains(btnId)) {
        Private::ButtonInfo info;
        info.index = btnId;
        info.icon = icon;
        info.toolTip = toolTip;
        info.matchRole = Qt::UserRole + 1;
        info.displayColumn = -1;
        info.itemSide = ItemRightSide;
        info.itemDisplayModes = DisplayOnDetection;
        d->m_btnInfos.insert(btnId, info);
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

    if (srcBtnInfo != nullptr) {
        if (dstBtnInfo != nullptr) {
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
    return btnInfo != nullptr ? btnInfo->matchRole : -1;
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
    return btnInfo != nullptr ? btnInfo->matchData : QVariant();
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
void ItemViewButtons::setButtonDetection(
        int btnId, int matchRole, const QVariant &matchData)
{
    Private::ButtonInfo* btnInfo = d->mutableButtonInfo(btnId);
    if (btnInfo != nullptr) {
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
    return btnInfo != nullptr ? btnInfo->displayColumn : -1;
}

void ItemViewButtons::setButtonDisplayColumn(int btnId, int col)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::displayColumn,
                d->mutableButtonInfo(btnId),
                col);
}

/*!
 * \brief Side in the item's cell where the button is displayed
 * \param btnId Index of the button
 * \returns -1 If button does not exist
 */
int ItemViewButtons::buttonItemSide(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->itemSide : -1;
}

void ItemViewButtons::setButtonItemSide(int btnId, ItemSide side)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::itemSide,
                d->mutableButtonInfo(btnId),
                side);
}

/*!
 * \brief Display modes of the button
 * \param btnId Index of the button
 * \returns DisplayModes() If button does not exist
 */
ItemViewButtons::DisplayModes ItemViewButtons::buttonDisplayModes(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->itemDisplayModes : DisplayModes();
}

void ItemViewButtons::setButtonDisplayModes(int btnId, DisplayModes modes)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::itemDisplayModes,
                d->mutableButtonInfo(btnId),
                modes);
}

/*!
 * \brief Icon of the button
 * \param btnId Index of the button
 * \returns QIcon() If button does not exist
 */
QIcon ItemViewButtons::buttonIcon(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->icon : QIcon();
}

void ItemViewButtons::setButtonIcon(int btnId, const QIcon &icon)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::icon,
                d->mutableButtonInfo(btnId),
                icon);
}

/*!
 * \brief Icon size of the button
 * \param btnId Index of the button
 * \returns QSize() If button does not exist
 */
QSize ItemViewButtons::buttonIconSize(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->iconSize : QSize();
}

void ItemViewButtons::setButtonIconSize(int btnId, const QSize &size)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::iconSize,
                d->mutableButtonInfo(btnId),
                size);
}

/*!
 * \brief Tool-tip of the button
 * \param btnId Index of the button
 * \returns QString() If button does not exist
 */
QString ItemViewButtons::buttonToolTip(int btnId) const
{
    const Private::ButtonInfo* btnInfo = d->buttonInfo(btnId);
    return btnInfo != nullptr ? btnInfo->toolTip : QString();
}

void ItemViewButtons::setButtonToolTip(int btnId, const QString &toolTip)
{
    cpp::checkedAssign(
                &Private::ButtonInfo::toolTip,
                d->mutableButtonInfo(btnId),
                toolTip);
}

/*! Install a delegate for the attached view item, allowing the button mechanism
 *  to work
 */
void ItemViewButtons::installDefaultItemDelegate()
{
    if (d->m_view != nullptr)
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
        QStyledItemDelegate *sourceDelegate, QObject *parent) const
{
    return new Private::ProxyItemDelegate(this, sourceDelegate, parent);
}

int ItemViewButtons::buttonAtModelIndex(const QModelIndex &index) const
{
    foreach (int id, d->m_btnInfos.keys()) {
        const Private::ButtonInfo* btnInfo = d->buttonInfo(id);
        if (btnInfo->matchRole < 0)
            return id;
        const QVariant modelItemData = index.data(btnInfo->matchRole);
        if ((!btnInfo->matchData.isNull() && btnInfo->matchData.isValid())
                && (!modelItemData.isNull() && modelItemData.isValid())
                && (btnInfo->matchData == modelItemData))
        {
            return id;
        }
    }
    return -1;
}

} // namespace qtgui
