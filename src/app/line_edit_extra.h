/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtGui/QIcon>
class QAbstractButton;
class QAction;
class QLineEdit;
class QMenu;

namespace Mayo {

// Provides additional left/right side buttons for QLineEdit widget
class LineEditExtra : public QObject {
    Q_OBJECT
public:
    enum class Side { Left, Right };

    LineEditExtra(QLineEdit* lineEdit = nullptr);

    QAbstractButton* button(Side side) const;

    QIcon buttonIcon(Side side) const;
    void setButtonIcon(Side side, const QIcon& icon);

    QMenu* buttonMenu(Side side) const;
    void setButtonMenu(Side side, QMenu* menu);

    void setButtonVisible(Side side, bool visible);
    bool isButtonVisible(Side side) const;

    // Set whether tabbing in will trigger the menu
    void setMenuTabFocusTrigger(Side side, bool v);
    bool hasMenuTabFocusTrigger(Side side) const;

    // Set if icon should be hidden when text is empty
    void setAutoHideButton(Side side, bool h);
    bool hasAutoHideButton(Side side) const;

    QLineEdit* widget() const { return m_widget; }

    bool eventFilter(QObject* object, QEvent* event) override;

signals:
    void leftButtonClicked();
    void rightButtonClicked();

private:
    class IconButton;
    IconButton* iconButton(Side side) const;

    static int toInt(Side side);
    static QAction* execMenuAtWidget(QMenu* menu, QWidget* widget);

    void handleIconClicked(Side side);
    void handleTextChanged(const QString& text);

    void updateMargins();
    void updateButtonPositions();

    QLineEdit* m_widget = nullptr;
    IconButton* m_iconButton[2] = {};
    QMenu* m_menu[2] = {};
    bool m_iconEnabled[2] = {};
    bool m_menuTabFocusTrigger[2] = {};
};

} // namespace Mayo
