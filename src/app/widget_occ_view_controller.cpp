/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "widget_occ_view_controller.h"
#include "widget_occ_view.h"
#include "qtwidgets_utils.h"
#include "qtgui_utils.h"
#include "theme.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QLineF>
#include <QtCore/QTimer>
#include <QtCore/QtGlobal>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#include <QtGui/QTouchEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QRubberBand>
#include <QtWidgets/QStyleFactory>
#include <algorithm>

namespace Mayo {

namespace {

static const QCursor& rotateCursor()
{
    static QCursor cursor;
    if (!cursor.pixmap()) {
        constexpr int cursorHotX = 6;
        constexpr int cursorHotY = 8;
        const QPixmap pixmap(":/images/graphics/rotate_cursor.png");
        cursor = QCursor(pixmap, cursorHotX, cursorHotY);
    }

    return cursor;
}

#if OCC_VERSION_HEX >= 0x070600
using RubberBandWidget_ParentType = QWidget;
#else
using RubberBandWidget_ParentType = QRubberBand;
#endif

class RubberBandWidget : public RubberBandWidget_ParentType {
public:
    explicit RubberBandWidget(QWidget* parent)
#if OCC_VERSION_HEX >= 0x070600
        : RubberBandWidget_ParentType(parent)
    {}
#else
        : RubberBandWidget_ParentType(QRubberBand::Rectangle, parent)
    {
        // QWidget::setStyle() is important, set to windows style will just draw
        // rectangle frame, otherwise will draw a solid rectangle.
        this->setStyle(QStyleFactory::create("windows"));
    }
#endif

protected:
#if OCC_VERSION_HEX >= 0x070600
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);

        const QColor lineColor = mayoTheme()->color(Theme::Color::RubberBandView3d_Line);
        QColor fillColor = mayoTheme()->color(Theme::Color::RubberBandView3d_Fill);
        fillColor.setAlpha(60);
        QPen pen = painter.pen();
        pen.setColor(lineColor);
        pen.setWidth(2);
        pen.setCapStyle(Qt::FlatCap);
        pen.setJoinStyle(Qt::MiterJoin);

        painter.setPen(pen);
        painter.setBrush(fillColor);
        painter.drawRect(this->rect().adjusted(1, 1, -1, -1));
    }
#endif
};

// Returns the positions of the touch points not yet released for 'event'
static std::vector<QPointF> activeTouchPositions(const QTouchEvent* event)
{
    std::vector<QPointF> vecPos;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    for (const QEventPoint& touchPnt : event->points()) {
        if (touchPnt.state() != QEventPoint::State::Released)
            vecPos.push_back(touchPnt.position());
    }
#else
    for (const QTouchEvent::TouchPoint& touchPnt : event->touchPoints()) {
        if (touchPnt.state() != Qt::TouchPointReleased)
            vecPos.push_back(touchPnt.pos());
    }
#endif
    return vecPos;
}

} // namespace

WidgetOccViewController::WidgetOccViewController(IWidgetOccView* occView)
    : QObject(occView->widget()),
      V3dViewController(occView->v3dView()),
      m_occView(occView),
      m_navigStyle(View3dNavigationStyle::Catia),
      m_actionMatcher(createActionMatcher(m_navigStyle, m_inputSequence))
{
    m_occView->widget()->installEventFilter(this);
    m_inputSequence.setPrePushCallback([=](Input in) { m_actionMatcher->onInputPrePush(in); });
    m_inputSequence.setPreReleaseCallback([=](Input in) { m_actionMatcher->onInputPreRelease(in); });
    m_inputSequence.setClearCallback([=] { m_actionMatcher->onInputCleared(); });

    if (QtWidgetsUtils::isTouchSessionActive()) {
        // In touch sessions the keyboard is not grabbed on mouse hover, so the view widget must
        // be able to receive focus to handle key events(space, shift, ...)
        m_occView->widget()->setFocusPolicy(Qt::StrongFocus);
    }

    m_touchLongPressTimer = new QTimer(this);
    m_touchLongPressTimer->setSingleShot(true);
    m_touchLongPressTimer->setInterval(700);
    QObject::connect(m_touchLongPressTimer, &QTimer::timeout, this, [=]{
        if (m_touchGesture.isActive && !m_touchGesture.isTwoFinger && !m_touchGesture.hasMoved) {
            // Freeze the gesture so that neither rotation nor tap-selection occurs after the
            // long-press, the finger can be released safely
            m_touchGesture.hasMoved = true;
            const Position pos = toPosition(m_touchGesture.pressPos.toPoint());
            emit this->touchLongPressRequested(pos.x, pos.y);
        }
    });
}

WidgetOccViewController::~WidgetOccViewController()
{
}

bool WidgetOccViewController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_occView->widget())
        return false;

    if (event->type() == QEvent::Enter) {
        m_inputSequence.clear();
        // Grab keyboard on mouse hover to handle key events(space, shift, ...). But in touch
        // sessions this is undesirable: hover doesn't exist for touch input and a keyboard grab
        // interferes with the virtual keyboard. In that case key events are delivered to the view
        // widget when it holds focus(focus is given to the widget on tap)
        if (!QtWidgetsUtils::isTouchSessionActive())
            m_occView->widget()->grabKeyboard();
        return false;
    }
    else if (event->type() == QEvent::Leave) {
        if (!QtWidgetsUtils::isTouchSessionActive())
            m_occView->widget()->releaseKeyboard();
        return false;
    }

    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        // Consume touch events so that Qt does not also synthesize mouse events from the first touch
        // point(this would conflict with the two fingers pinch gesture)
        this->handleTouch(static_cast<const QTouchEvent*>(event));
        return true;
    default:
        break;
    } // end switch

    this->handleEvent(event);
    return false;
}

void WidgetOccViewController::setNavigationStyle(View3dNavigationStyle style)
{
    m_navigStyle = style;
    m_inputSequence.clear();
    m_actionMatcher = createActionMatcher(style, m_inputSequence);
}

void WidgetOccViewController::redrawView()
{
    //V3dViewController::redrawView();
    m_occView->redraw();
}

void WidgetOccViewController::startDynamicAction(V3dViewController::DynamicAction action)
{
    if (action == DynamicAction::Rotation)
        this->setViewCursor(rotateCursor());
    else if (action == DynamicAction::Panning)
        this->setViewCursor(Qt::SizeAllCursor);
    else if (action == DynamicAction::Zoom)
        this->setViewCursor(Qt::SizeVerCursor);
    else if (action == DynamicAction::WindowZoom)
        this->setViewCursor(Qt::SizeBDiagCursor);

    V3dViewController::startDynamicAction(action);
}

void WidgetOccViewController::stopDynamicAction()
{
    this->setViewCursor(Qt::ArrowCursor);
    V3dViewController::stopDynamicAction();
}

void WidgetOccViewController::setViewCursor(const QCursor &cursor)
{
    if (m_occView->widget())
        m_occView->widget()->setCursor(cursor);
}

struct WidgetOccViewController::RubberBand : public V3dViewController::IRubberBand {
    explicit RubberBand(QWidget* parent)
        : m_rubberBand(parent)
    {
    }

    void updateGeometry(int x, int y, int width, int height) override {
        m_rubberBand.setGeometry(x, y, width, height);
    }

    void setVisible(bool on) override {
        m_rubberBand.setVisible(on);
    }

private:
    RubberBandWidget m_rubberBand;
};

std::unique_ptr<V3dViewController::IRubberBand> WidgetOccViewController::createRubberBand()
{
    return std::make_unique<RubberBand>(m_occView->widget());
}

void WidgetOccViewController::handleEvent(const QEvent* event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        this->handleKeyPress(static_cast<const QKeyEvent*>(event));
        break;
    case QEvent::KeyRelease:
        this->handleKeyRelease(static_cast<const QKeyEvent*>(event));
        break;
    case QEvent::MouseButtonPress:
        this->handleMouseButtonPress(static_cast<const QMouseEvent*>(event));
        break;
    case QEvent::MouseMove:
        this->handleMouseMove(static_cast<const QMouseEvent*>(event));
        break;
    case QEvent::MouseButtonRelease:
        this->handleMouseButtonRelease(static_cast<const QMouseEvent*>(event));
        break;
    case QEvent::Wheel:
        this->handleMouseWheel(static_cast<const QWheelEvent*>(event));
        break;
    default:
        break;
    } // end switch
}

void WidgetOccViewController::handleKeyPress(const QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    m_inputSequence.push(event->key());
    if (m_inputSequence.equal({ Qt::Key_Space }))
        this->startInstantZoom(toPosition(m_occView->widget()->mapFromGlobal(QCursor::pos())));

    if (m_inputSequence.equal({ Qt::Key_Shift }) && !this->hasCurrentDynamicAction())
        this->signalMultiSelectionToggled.send(true);
}

void WidgetOccViewController::handleKeyRelease(const QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    m_inputSequence.release(event->key());
    if (!m_inputSequence.equal({}))
        return;

    if (m_inputSequence.lastInput() == Qt::Key_Space && this->currentDynamicAction() == DynamicAction::InstantZoom)
        this->stopInstantZoom();

    if (m_inputSequence.lastInput() == Qt::Key_Shift && !this->hasCurrentDynamicAction())
        this->signalMultiSelectionToggled.send(false);
}

void WidgetOccViewController::handleMouseButtonPress(const QMouseEvent* event)
{
    m_inputSequence.push(event->button());
    const QPoint currPos = m_occView->widget()->mapFromGlobal(QtGuiUtils::globalPosition(event));
    m_prevPos = toPosition(currPos);
}

void WidgetOccViewController::handleMouseMove(const QMouseEvent* event)
{
    const Position currPos = toPosition(m_occView->widget()->mapFromGlobal(QtGuiUtils::globalPosition(event)));
    const Position prevPos = m_prevPos;
    m_prevPos = currPos;
    if (m_actionMatcher->matchRotation())
        this->rotation(currPos);
    else if (m_actionMatcher->matchPan())
        this->pan(prevPos, currPos);
    else if (m_actionMatcher->matchZoom())
        this->zoom(prevPos, currPos);
    else if (m_actionMatcher->matchWindowZoom())
        this->windowZoomRubberBand(currPos);
    else
        this->signalMouseMoved.send(currPos.x, currPos.y);
}

void WidgetOccViewController::handleMouseButtonRelease(const QMouseEvent* event)
{
    auto fnOccMouseBtn = [](Qt::MouseButton btn) -> Aspect_VKeyMouse {
        switch (btn) {
        case Qt::NoButton: return Aspect_VKeyMouse_NONE;
        case Qt::LeftButton: return Aspect_VKeyMouse_LeftButton;
        case Qt::RightButton: return Aspect_VKeyMouse_RightButton;
        case Qt::MiddleButton: return Aspect_VKeyMouse_MiddleButton;
        default: return Aspect_VKeyMouse_UNKNOWN;
        }
    };

    m_inputSequence.release(event->button());
    const bool hadDynamicAction = this->hasCurrentDynamicAction();
    if (this->isWindowZoomingStarted())
        this->windowZoom(toPosition(m_occView->widget()->mapFromGlobal(QtGuiUtils::globalPosition(event))));

    this->stopDynamicAction();
    if (!hadDynamicAction)
        this->signalMouseButtonClicked.send(fnOccMouseBtn(event->button()));
}

void WidgetOccViewController::handleMouseWheel(const QWheelEvent* event)
{
    const QPoint delta = event->angleDelta();
    const Position pos = toPosition(m_occView->widget()->mapFromGlobal(QtGuiUtils::globalPosition(event)));
    const double zoomDelta = delta.y() != 0 ? delta.y() : delta.x();
    this->zoomAt(pos, zoomDelta / 120.); // Qt returns delta as a number of ticks out of 120
}

void WidgetOccViewController::handleTouch(const QTouchEvent* event)
{
    // Touch screen gestures:
    //   * 2 fingers    -> pinch to zoom + drag to pan(zoom anchored on the fingers centroid)
    //   * 1 finger     -> rotation
    //   * tap          -> highlight entity under the finger, second tap selects
    //   * long-press   -> context menu(touchLongPressRequested signal)
    // Touch events are consumed by the event filter(see eventFilter()) so that Qt does not also
    // synthesize mouse events from the first touch point
    const std::vector<QPointF> vecPos = activeTouchPositions(event);
    const QEvent::Type eventType = event->type();

    if (eventType == QEvent::TouchBegin) {
        m_touchGesture = TouchGesture{};
        m_touchGesture.isActive = true;
        m_touchGesture.pressPos = vecPos.empty() ? QPointF() : vecPos.front();
        m_touchGesture.timer.start();
        // Key events(space, shift, ...) are handled by the view widget when it holds focus
        m_occView->widget()->setFocus(Qt::OtherFocusReason);
        m_touchLongPressTimer->start();
    }

    if (eventType == QEvent::TouchEnd || eventType == QEvent::TouchCancel) {
        m_touchLongPressTimer->stop();
        const bool isTap =
            eventType == QEvent::TouchEnd
            && m_touchGesture.isActive
            && !m_touchGesture.isTwoFinger
            && !m_touchGesture.hasMoved
            && m_touchGesture.timer.elapsed() < QApplication::startDragTime()
        ;
        if (isTap) {
            // Update the highlighted object(like the mouse move path) then select it(like the
            // mouse click path)
            const Position pos = toPosition(m_touchGesture.pressPos.toPoint());
            this->signalMouseMoved.send(pos.x, pos.y);
            this->signalMouseButtonClicked.send(Aspect_VKeyMouse_LeftButton);
        }

        this->stopDynamicAction();
        m_touchGesture = TouchGesture{};
        return;
    }

    if (vecPos.size() == 2) {
        const QPointF centroid = (vecPos[0] + vecPos[1]) / 2.;
        const double spread = QLineF(vecPos[0], vecPos[1]).length();
        if (!m_touchGesture.isTwoFinger) {
            // Transition from one to two fingers: cancel the ongoing single-finger action, if any
            m_touchLongPressTimer->stop();
            if (this->hasCurrentDynamicAction())
                this->stopDynamicAction();

            m_touchGesture.isTwoFinger = true;
            m_touchGesture.hasMoved = true;
        }
        else {
            const double factor =
                m_touchGesture.prevSpread > 0. ? spread / m_touchGesture.prevSpread : 1.;
            this->zoomByFactor(toPosition(centroid.toPoint()), factor);
            // Two-finger drag pans the view(move of the fingers centroid)
            this->pan(
                toPosition(m_touchGesture.prevCentroid.toPoint()),
                toPosition(centroid.toPoint())
            );
        }

        m_touchGesture.prevCentroid = centroid;
        m_touchGesture.prevSpread = spread;
    }
    else if (vecPos.size() == 1 && !m_touchGesture.isTwoFinger) {
        const QPointF currPos = vecPos.front();
        if (!m_touchGesture.hasMoved) {
            const double dragDist = QLineF(m_touchGesture.pressPos, currPos).length();
            if (dragDist > QApplication::startDragDistance()) {
                m_touchGesture.hasMoved = true;
                m_touchLongPressTimer->stop();
                this->rotation(toPosition(m_touchGesture.pressPos.toPoint()));
            }
        }

        if (m_touchGesture.hasMoved)
            this->rotation(toPosition(currPos.toPoint()));
    }
}

class WidgetOccViewController::Mayo_ActionMatcher : public ActionMatcher {
public:
    // Inherit ActionMatcher constructor
    using ActionMatcher::ActionMatcher;

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::LeftButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::RightButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::LeftButton, Qt::RightButton })
                || this->inputs.equal({ Qt::RightButton, Qt::LeftButton });
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

class WidgetOccViewController::Catia_ActionMatcher : public ActionMatcher {
public:
    explicit Catia_ActionMatcher(const InputSequence& seq) : ActionMatcher(seq) {
        m_timer.start();
    }

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton, Qt::LeftButton })
                || this->inputs.equal({ Qt::MiddleButton, Qt::RightButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::MiddleButton }) && !this->matchZoom();
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::MiddleButton })
                && m_beforeLastOp == InputSequence::Operation::Push
                && (m_beforeLastInput == Qt::LeftButton || m_beforeLastInput == Qt::RightButton)
                && this->inputs.lastOperation() == InputSequence::Operation::Release
                && this->inputs.lastInput() == m_beforeLastInput
                && (m_lastTimestamp_ms - m_beforeLastTimestamp_ms) < 750
                ;
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::MiddleButton });
    }

    void onInputPrePush(Input /*in*/) override {
        this->recordBeforeLastOperation();
    }

    void onInputPreRelease(Input /*in*/) override {
        this->recordBeforeLastOperation();
    }

    void onInputCleared() override {
        m_beforeLastOp = InputSequence::Operation::None;
        m_beforeLastInput = -1;
        m_beforeLastTimestamp_ms = 0;
        m_lastTimestamp_ms = 0;
        m_timer.restart();
    }

private:
    void recordBeforeLastOperation() {
        m_beforeLastOp = this->inputs.lastOperation();
        m_beforeLastInput = this->inputs.lastInput();
        m_beforeLastTimestamp_ms = m_lastTimestamp_ms;
        m_lastTimestamp_ms = m_timer.elapsed();
    }

    InputSequence::Operation m_beforeLastOp = InputSequence::Operation::None;
    Input m_beforeLastInput = -1;
    int64_t m_beforeLastTimestamp_ms = 0;
    int64_t m_lastTimestamp_ms = 0;
    QElapsedTimer m_timer;
};

class WidgetOccViewController::SolidWorks_ActionMatcher : public ActionMatcher {
public:
    // Inherit ActionMatcher constructor
    using ActionMatcher::ActionMatcher;

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::MiddleButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::Key_Shift, Qt::MiddleButton });
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

class WidgetOccViewController::Unigraphics_ActionMatcher : public ActionMatcher {
public:
    // Inherit ActionMatcher constructor
    using ActionMatcher::ActionMatcher;

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::MiddleButton, Qt::RightButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::MiddleButton, Qt::LeftButton });;
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

class WidgetOccViewController::ProEngineer_ActionMatcher : public ActionMatcher {
public:
    // Inherit ActionMatcher constructor
    using ActionMatcher::ActionMatcher;

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::Key_Shift, Qt::MiddleButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::MiddleButton });
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

std::unique_ptr<WidgetOccViewController::ActionMatcher>
WidgetOccViewController::createActionMatcher(View3dNavigationStyle style, const InputSequence& seq)
{
    switch(style) {
    case View3dNavigationStyle::Mayo: return std::make_unique<Mayo_ActionMatcher>(seq);
    case View3dNavigationStyle::Catia: return std::make_unique<Catia_ActionMatcher>(seq);
    case View3dNavigationStyle::SolidWorks: return std::make_unique<SolidWorks_ActionMatcher>(seq);
    case View3dNavigationStyle::Unigraphics: return std::make_unique<Unigraphics_ActionMatcher>(seq);
    case View3dNavigationStyle::ProEngineer: return std::make_unique<ProEngineer_ActionMatcher>(seq);
    }
    return {};
}

void WidgetOccViewController::InputSequence::push(Input in)
{
    auto itFound = std::find(m_inputs.cbegin(), m_inputs.cend(), in);
    if (itFound != m_inputs.cend())
        m_inputs.erase(itFound);

    if (m_fnPrePushCallback)
        m_fnPrePushCallback(in);

    m_inputs.push_back(in);
    m_lastOperation = Operation::Push;
    m_lastInput = in;
}

void WidgetOccViewController::InputSequence::release(Input in)
{
    auto itRemoved = std::remove(m_inputs.begin(), m_inputs.end(), in);
    if (itRemoved != m_inputs.end()) {
        if (m_fnPreReleaseCallback)
            m_fnPreReleaseCallback(in);

        m_inputs.erase(itRemoved, m_inputs.end());
        m_lastOperation = Operation::Release;
        m_lastInput = in;
    }
}

void WidgetOccViewController::InputSequence::clear()
{
    m_inputs.clear();
    m_lastOperation = Operation::None;
    m_lastInput = -1;
    if (m_fnClearCallback)
        m_fnClearCallback();
}

bool WidgetOccViewController::InputSequence::equal(std::initializer_list<Input> other) const
{
    return std::equal(m_inputs.cbegin(), m_inputs.cend(), other.begin(), other.end());
}

} // namespace Mayo
