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

#pragma once

class QAbstractItemView;
class QIcon;
class QPainter;
class QStyledItemDelegate;
class QStyleOptionViewItem;

#include "gui.h"

#include <QtCore/QHash>
#include <QtCore/QModelIndex>
#include <QtCore/QObject>
#include <QtCore/QFlags>
#include <QtGui/QIcon>

namespace qtgui {

class QTTOOLS_GUI_EXPORT ItemViewButtons : public QObject
{
    Q_OBJECT

public:
    // Types
    enum ItemSide
    {
        ItemLeftSide,
        ItemRightSide
    };

    enum DisplayMode
    {
        DisplayOnDetection = 0x01,
        DisplayPermanent = 0x02,
        DisplayWhenItemSelected = 0x04
    };
    typedef QFlags<DisplayMode> DisplayModes;

    // Ctor & dtor
    ItemViewButtons(QAbstractItemView* view, QObject* parent = NULL);
    ~ItemViewButtons();

    // View control
    QAbstractItemView* itemView() const;

    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    void paint(
            QPainter* painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index) const;

    // Button management
    void addButton(
            int btnId,
            const QIcon& icon = QIcon(),
            const QString& toolTip = QString());
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
            QStyledItemDelegate *sourceDelegate,
            QObject* parent = NULL) const;

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

} // namespace qtgui

Q_DECLARE_OPERATORS_FOR_FLAGS(qtgui::ItemViewButtons::DisplayModes)
