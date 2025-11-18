/****************************************************************************
** Copyright (c) 2025, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "line_edit_extra.h"

#include <QtCore/QPropertyAnimation>
#include <QtGui/QKeyEvent>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyleOptionFocusRect>
#include <QtWidgets/QStylePainter>

#include <algorithm>

namespace Mayo {

// Provides button to be embedded in QLineEdit with LineEditExtra
class LineEditExtra::IconButton : public QAbstractButton {
    //Q_OBJECT
    //Q_PROPERTY(float iconOpacity READ iconOpacity WRITE setIconOpacity)
    //Q_PROPERTY(bool autoHide READ isAutoHide WRITE setAutoHide)
public:
    IconButton(QWidget* parent = nullptr);

    float iconOpacity() { return m_iconOpacity; }
    void setIconOpacity(float value);
    void animateShow(bool visible);

    void setAutoHide(bool on) { m_autoHide = on; }
    bool isAutoHide() const { return m_autoHide; }

    void setMenuIndicator(bool on) { m_hasMenuIndicator = on; }
    bool hasMenuIndicator() const { return m_hasMenuIndicator; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    float m_iconOpacity = 1.f;
    bool m_autoHide = false;
    bool m_hasMenuIndicator = false;
};

LineEditExtra::IconButton::IconButton(QWidget* parent)
    : QAbstractButton(parent)
{
    this->setCursor(Qt::ArrowCursor);
    this->setFocusPolicy(Qt::NoFocus);
}

void LineEditExtra::IconButton::setIconOpacity(float value)
{
    m_iconOpacity = value;
    this->update();
}

void LineEditExtra::IconButton::paintEvent(QPaintEvent *)
{
    const qreal pixelRatio = this->window()->windowHandle()->devicePixelRatio();
    QPixmap iconPixmap = this->icon().pixmap(
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            this->sizeHint(), pixelRatio, this->isEnabled() ? QIcon::Normal : QIcon::Disabled
#else
            this->sizeHint(), this->isEnabled() ? QIcon::Normal : QIcon::Disabled
#endif
        );

    QStylePainter painter(this);
    QRect pixmapRect(QPoint{}, iconPixmap.size() / iconPixmap.devicePixelRatio());
    pixmapRect.moveCenter(this->rect().center());

    if (m_autoHide)
        painter.setOpacity(m_iconOpacity);

    painter.drawPixmap(pixmapRect, iconPixmap);

    if (this->hasFocus()) {
        QStyleOptionFocusRect focusOption;
        focusOption.initFrom(this);
        focusOption.rect = pixmapRect;
#ifdef Q_OS_MAC
        focusOption.rect.adjust(-4, -4, 4, 4);
        painter.drawControl(QStyle::CE_FocusFrame, focusOption);
#else
        painter.drawPrimitive(QStyle::PE_FrameFocusRect, focusOption);
#endif
    }

    if (m_hasMenuIndicator) {
        QStyleOption option;
        option.initFrom(this);
        option.rect.setTopLeft({
            pixmapRect.left() + int(pixmapRect.width() * 0.75),
            pixmapRect.top() + int(pixmapRect.height() * 0.75)
        });
        option.rect.setSize({
            int(pixmapRect.width() / 2.5),
            int(pixmapRect.height() / 2.5)
        });
        painter.drawPrimitive(QStyle::PE_IndicatorArrowDown, option);
    }
}

void LineEditExtra::IconButton::animateShow(bool visible)
{
    auto animation = new QPropertyAnimation(this, "iconOpacity");
    animation->setDuration(160/*ms*/);
    animation->setEndValue(visible ? 1.0 : 0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

QSize LineEditExtra::IconButton::sizeHint() const
{
    // Find flags icon can be wider than 16px
    return this->icon().actualSize(QSize(32, 16));
}

void LineEditExtra::IconButton::keyPressEvent(QKeyEvent* event)
{
    QAbstractButton::keyPressEvent(event);
    if (!event->modifiers() && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
        this->click();

    // Do not forward to line edit
    event->accept();
}

void LineEditExtra::IconButton::keyReleaseEvent(QKeyEvent* event)
{
    QAbstractButton::keyReleaseEvent(event);
    // Do not forward to line edit
    event->accept();
}

LineEditExtra::LineEditExtra(QLineEdit* lineEdit)
    : QObject(lineEdit),
    m_widget(lineEdit)
{
    for (int i = 0; i < 2; ++i) {
        auto btn = new IconButton(lineEdit);
        btn->installEventFilter(this);
        btn->hide();
        btn->setAutoHide(false);
        m_iconButton[i] = btn;
    }

    lineEdit->installEventFilter(this);
    this->updateMargins();

    QObject::connect(
        m_iconButton[0], &QAbstractButton::clicked, this, [=]{ this->handleIconClicked(Side::Left); }
    );
    QObject::connect(
        m_iconButton[1], &QAbstractButton::clicked, this, [=]{ this->handleIconClicked(Side::Right); }
    );
    QObject::connect(
        lineEdit, &QLineEdit::textChanged, this, &LineEditExtra::handleTextChanged
    );
#if 0
    QObject::connect(&d->m_completionShortcut, &QShortcut::activated, this, [this] {
        if (!completer())
            return;
        completer()->setCompletionPrefix(text().left(cursorPosition()));
        completer()->complete();
    });
#endif
}

QAbstractButton* LineEditExtra::button(Side side) const
{
    return this->iconButton(side);
}

QIcon LineEditExtra::buttonIcon(Side side) const
{
    return this->button(side)->icon();
}

void LineEditExtra::setButtonIcon(Side side, const QIcon& icon)
{
    this->button(side)->setIcon(icon);
    this->updateMargins();
    this->updateButtonPositions();
    m_widget->update();
}

QMenu* LineEditExtra::buttonMenu(Side side) const
{
    return m_menu[toInt(side)];
}

void LineEditExtra::setButtonMenu(Side side, QMenu* menu)
{
    m_menu[toInt(side)] = menu;
    this->iconButton(side)->setIconOpacity(1.);
    this->iconButton(side)->setMenuIndicator(true);
    menu->installEventFilter(this);
}

void LineEditExtra::setButtonVisible(Side side, bool on)
{
    this->iconButton(side)->setVisible(on);
    m_iconEnabled[toInt(side)] = on;
    this->updateMargins();
}

bool LineEditExtra::isButtonVisible(Side side) const
{
    return m_iconEnabled[toInt(side)];
}

bool LineEditExtra::hasMenuTabFocusTrigger(Side side) const
{
    return m_menuTabFocusTrigger[toInt(side)];
}

void LineEditExtra::setMenuTabFocusTrigger(Side side, bool on)
{
    if (m_menuTabFocusTrigger[toInt(side)] == on)
        return;

    m_menuTabFocusTrigger[toInt(side)] = on;
    this->iconButton(side)->setFocusPolicy(on ? Qt::TabFocus : Qt::NoFocus);
}

bool LineEditExtra::isButtonAutoHide(Side side) const
{
    return this->iconButton(side)->isAutoHide();
}

void LineEditExtra::setButtonAutoHide(Side side, bool on)
{
    auto iconBtn = this->iconButton(side);
    iconBtn->setAutoHide(on);
    if (on)
        iconBtn->setIconOpacity(m_widget->text().isEmpty() ?  0.0 : 1.0);
    else
        iconBtn->setIconOpacity(1.0);
}

bool LineEditExtra::eventFilter(QObject* object, QEvent* event)
{
    // Handle specific events for target QLineEdit
    if (object == m_widget) {
        if (event->type() == QEvent::Resize) {
            this->updateButtonPositions();
            return true;
        }
    }

    // Find if watched object is an icon button
    int btnIndex = -1;
    for (int i = 0; i < 2; ++i) {
        if (object == m_iconButton[i]) {
            btnIndex = i;
            break;
        }
    }

    // Handle specific events for icon button
    if (btnIndex != -1) {
        if (event->type() == QEvent::FocusIn
            && m_menuTabFocusTrigger[btnIndex] && m_menu[btnIndex])
        {
            m_widget->setFocus();
            execMenuAtWidget(m_menu[btnIndex], m_iconButton[btnIndex]);
            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

LineEditExtra::IconButton* LineEditExtra::iconButton(Side side) const
{
    return m_iconButton[toInt(side)];
}

QAction* LineEditExtra::execMenuAtWidget(QMenu* menu, QWidget* widget)
{
    QPoint p;
    const QRect screen = widget->screen()->availableGeometry();
    const QSize sh = menu->sizeHint();
    const QRect rect = widget->rect();
    if (widget->isRightToLeft()) {
        if (widget->mapToGlobal(QPoint{0, rect.bottom()}).y() + sh.height() <= screen.height())
            p = widget->mapToGlobal(rect.bottomRight());
        else
            p = widget->mapToGlobal(rect.topRight() - QPoint{0, sh.height()});

        p.rx() -= sh.width();
    }
    else {
        if (widget->mapToGlobal(QPoint{0, rect.bottom()}).y() + sh.height() <= screen.height())
            p = widget->mapToGlobal(rect.bottomLeft());
        else
            p = widget->mapToGlobal(rect.topLeft() - QPoint{0, sh.height()});
    }

    p.rx() = std::max(screen.left(), std::min(p.x(), screen.right() - sh.width()));
    p.ry() += 1;

    return menu->exec(p);
}

void LineEditExtra::handleIconClicked(Side side)
{
    if (this->buttonMenu(side)) {
        execMenuAtWidget(this->buttonMenu(side), this->button(side));
    } else {
        //emit buttonClicked(side);
        if (side == Side::Left)
            emit leftButtonClicked();
        else if (side == Side::Right)
            emit rightButtonClicked();
    }
}

void LineEditExtra::handleTextChanged(const QString& text)
{
    for (IconButton* btn : m_iconButton) {
        if (btn->isAutoHide())
            btn->setIconOpacity(text.isEmpty() ? 0.f : 1.f);
    }
}

void LineEditExtra::updateMargins()
{
    const bool leftToRight = m_widget->layoutDirection() == Qt::LeftToRight;
    const Side realLeft = leftToRight ? Side::Left : Side::Right;
    const Side realRight = leftToRight ? Side::Right : Side::Left;

    int leftMargin = this->button(realLeft)->sizeHint().width() + 8;
    int rightMargin = this->button(realRight)->sizeHint().width() + 8;
    // KDE does not reserve space for the highlight color
    if (m_widget->style()->inherits("OxygenStyle")) {
        leftMargin = std::max(24, leftMargin);
        rightMargin = std::max(24, rightMargin);
    }

    const QMargins margins(
        (m_iconEnabled[toInt(realLeft)] ? leftMargin : 0), 0,
        (m_iconEnabled[toInt(realRight)] ? rightMargin : 0), 0
    );
    m_widget->setTextMargins(margins);
}

void LineEditExtra::updateButtonPositions()
{
    const QRect contentRect = m_widget->rect();
    for (int i = 0; i < 2; ++i) {
        Side iconpos = static_cast<Side>(i);
        if (m_widget->layoutDirection() == Qt::RightToLeft)
            iconpos = (iconpos == Side::Left ? Side::Right : Side::Left);

        if (iconpos == Side::Right) {
            const int iconOffset = m_widget->textMargins().right() + 4;
            m_iconButton[i]->setGeometry(contentRect.adjusted(m_widget->width() - iconOffset, 0, 0, 0));
        } else {
            const int iconOffset = m_widget->textMargins().left() + 4;
            m_iconButton[i]->setGeometry(contentRect.adjusted(0, 0, -m_widget->width() + iconOffset, 0));
        }
    }
}

int LineEditExtra::toInt(Side side)
{
    if (side == Side::Left)
        return 0;
    else
        return 1;
}

} // namespace Mayo

