/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_occ_view_controller.h"
#include "widget_occ_view.h"
#include "qtgui_utils.h"
#include "theme.h"

#include <QtCore/QDebug>
#include <QtCore/QElapsedTimer>
#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QRubberBand>
#include <QtWidgets/QStyleFactory>
#include <algorithm>

namespace Mayo {

namespace Internal {

static const QCursor& rotateCursor()
{
    static QCursor cursor;
    if (!cursor.bitmap()) {
        constexpr int cursorWidth = 16;
        constexpr int cursorHeight = 16;
        constexpr int cursorHotX = 6;
        constexpr int cursorHotY = 8;

        static unsigned char cursorBitmap[] = {
            0xf0, 0xef, 0x18, 0xb8, 0x0c, 0x90, 0xe4, 0x83,
            0x34, 0x86, 0x1c, 0x83, 0x00, 0x81, 0x00, 0xff,
            0xff, 0x00, 0x81, 0x00, 0xc1, 0x38, 0x61, 0x2c,
            0xc1, 0x27, 0x09, 0x30, 0x1d, 0x18, 0xf7, 0x0f
        };
        static unsigned char cursorMaskBitmap[] = {
            0xf0, 0xef, 0xf8, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0x3c, 0xfe, 0x1c, 0xff, 0x00, 0xff, 0x00,
            0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x38, 0x7f, 0x3c, 0xff, 0x3f, 0xff, 0x3f, 0xff, 0x1f,
            0xf7, 0x0f
        };

        const QBitmap cursorBmp = QBitmap::fromData({ cursorWidth, cursorHeight }, cursorBitmap);
        const QBitmap maskBmp = QBitmap::fromData({ cursorWidth, cursorHeight }, cursorMaskBitmap);
        const QCursor tempCursor(cursorBmp, maskBmp, cursorHotX, cursorHotY);
        cursor = std::move(tempCursor);
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
    RubberBandWidget(QWidget* parent)
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

} // namespace Internal

WidgetOccViewController::WidgetOccViewController(IWidgetOccView* occView)
    : QObject(occView->widget()),
      V3dViewController(occView->v3dView()),
      m_occView(occView),
      m_navigStyle(View3dNavigationStyle::Catia),
      m_actionMatcher(createActionMatcher(m_navigStyle, &m_inputSequence))
{
    m_occView->widget()->installEventFilter(this);
    m_inputSequence.setPrePushCallback([=](Input in) { m_actionMatcher->onInputPrePush(in); });
    m_inputSequence.setPreReleaseCallback([=](Input in) { m_actionMatcher->onInputPreRelease(in); });
    m_inputSequence.setClearCallback([=] { m_actionMatcher->onInputCleared(); });
}

bool WidgetOccViewController::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != m_occView->widget())
        return false;

    if (event->type() == QEvent::Enter) {
        m_inputSequence.clear();
        m_occView->widget()->grabKeyboard();
        return false;
    }
    else if (event->type() == QEvent::Leave) {
        m_occView->widget()->releaseKeyboard();
        return false;
    }

    this->handleEvent(event);
    return false;
}

void WidgetOccViewController::setNavigationStyle(View3dNavigationStyle style)
{
    m_navigStyle = style;
    m_inputSequence.clear();
    m_actionMatcher = createActionMatcher(style, &m_inputSequence);
}

void WidgetOccViewController::redrawView()
{
    //V3dViewController::redrawView();
    m_occView->redraw();
}

void WidgetOccViewController::startDynamicAction(V3dViewController::DynamicAction action)
{
    if (action == DynamicAction::Rotation)
        this->setViewCursor(Internal::rotateCursor());
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
    RubberBand(QWidget* parent)
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
    Internal::RubberBandWidget m_rubberBand;
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
    if (delta.y() > 0 || (delta.y() == 0 && delta.x() > 0))
        this->zoomIn();
    else
        this->zoomOut();
}

class WidgetOccViewController::Mayo_ActionMatcher : public ActionMatcher {
public:
    Mayo_ActionMatcher(const InputSequence* seq) : ActionMatcher(seq) {}

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
    Catia_ActionMatcher(const InputSequence* seq) : ActionMatcher(seq) {
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
    SolidWorks_ActionMatcher(const InputSequence* seq) : ActionMatcher(seq) {}

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::MiddleButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::Key_Shift, Qt::MiddleButton });;
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

class WidgetOccViewController::Unigraphics_ActionMatcher : public ActionMatcher {
public:
    Unigraphics_ActionMatcher(const InputSequence* seq) : ActionMatcher(seq) {}

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
    ProEngineer_ActionMatcher(const InputSequence* seq) : ActionMatcher(seq) {}

    bool matchRotation() const override {
        return this->inputs.equal({ Qt::MiddleButton });
    }

    bool matchPan() const override {
        return this->inputs.equal({ Qt::Key_Shift, Qt::MiddleButton });
    }

    bool matchZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::MiddleButton });;
    }

    bool matchWindowZoom() const override {
        return this->inputs.equal({ Qt::Key_Control, Qt::LeftButton });
    }
};

std::unique_ptr<WidgetOccViewController::ActionMatcher>
WidgetOccViewController::createActionMatcher(View3dNavigationStyle style, const InputSequence* seq)
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
