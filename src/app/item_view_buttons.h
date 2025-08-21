/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <QtGui/QIcon>
class QAbstractItemView;
class QIcon;
class QPainter;
class QStyledItemDelegate;
class QStyleOptionViewItem;

namespace Mayo {

/*!
 * \class ItemViewButtons
 * \brief Provides buttons integrated to items displayed by QAbstractItemView
 *
 * ItemViewButtons allows to add buttons inside any QAbstractItemView without subclassing the
 * item-view class.
 *
 * It only requires that its paint() method is called whenever any view item has to be drawn. If you
 * have a custom delegate(eg. a subclass of QStyledItemDelegate) then just call at some point
 * ItemViewButtons::paint() inside the delegate's paint() method :
 * \code
 * void MyCustomDelegate::paint(
 *         QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index
 *     ) const
 * {
 *   QStyledItemDelegate::paint(painter, option, index);
 *   // Specific paint operations ...
 *   m_itemViewBtns->paint(painter, option, index);
 * }
 * \endcode
 *
 * If you do not want to modify your delegate class then createProxyItemDelegate() might be the
 * right option : this will create a new delegate around yours with the paint() method correctly
 * called.
 *
 * If the item-view does not use any delegate then just call installDefaultItemDelegate()
 *
 * ItemViewButtons notifies any button click with signal buttonClicked()
 *
 */
class ItemViewButtons : public QObject {
    Q_OBJECT
public:
    // Types
    enum ItemSide {
        ItemLeftSide,
        ItemRightSide
    };

    enum DisplayMode {
        DisplayOnDetection = 0x01,
        DisplayPermanent = 0x02,
        DisplayWhenItemSelected = 0x04
    };
    using DisplayModes = QFlags<DisplayMode>;

    // Ctor & dtor
    ItemViewButtons(QAbstractItemView* view, QObject* parent = nullptr);
    ~ItemViewButtons();

    // View control
    QAbstractItemView* itemView() const;

    bool eventFilter(QObject* object, QEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    // Button management
    void addButton(int btnId, const QIcon& icon = {}, const QString& toolTip = {});
    void copyButtonProperties(int srcBtnId, int dstBtnId);

    int buttonDetectionMatchRole(int btnId) const;
    QVariant buttonDetectionMatchData(int btnId) const;
    void setButtonDetection(int btnId, int matchRole, const QVariant& matchData);

    int buttonDisplayColumn(int btnId) const;
    void setButtonDisplayColumn(int btnId, int col = -1);

    int buttonItemSide(int btnId) const;
    void setButtonItemSide(int btnId, ItemSide side);

    DisplayModes buttonDisplayModes(int btnId) const;
    void setButtonDisplayModes(int btnId, DisplayModes modes);

    QIcon buttonIcon(int btnId) const;
    void setButtonIcon(int btnId, const QIcon& icon);

    QSize buttonIconSize(int btnId) const;
    void setButtonIconSize(int btnId, const QSize& size);

    QString buttonToolTip(int btnId) const;
    void setButtonToolTip(int btnId, const QString& toolTip);

    // Delegates
    void installDefaultItemDelegate();
    QStyledItemDelegate* createProxyItemDelegate(
            QStyledItemDelegate* sourceDelegate, QObject* parent = nullptr
        ) const;

signals:
    void buttonClicked(int btnId, const QModelIndex& index);

public slots:
    void reset();

protected:
    virtual int buttonAtModelIndex(const QModelIndex& index) const;

private:
    class Private;
    friend class Private;
    Private* const d;
};

} // namespace Mayo

Q_DECLARE_OPERATORS_FOR_FLAGS(Mayo::ItemViewButtons::DisplayModes)
