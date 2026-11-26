/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../gui/v3d_view_controller.h"
#include "view3d_navigation_style.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QPointF>
#include <QtCore/QTimer>
#include <gsl/span>
#include <functional>
#include <memory>
#include <vector>
class QCursor;
class QKeyEvent;
class QMouseEvent;
class QRubberBand;
class QTouchEvent;
class QWheelEvent;

namespace Mayo {

class IWidgetOccView;

class WidgetOccViewController : public QObject, public V3dViewController {
    Q_OBJECT
public:
    explicit WidgetOccViewController(IWidgetOccView* occView = nullptr);
    ~WidgetOccViewController();

    bool eventFilter(QObject* watched, QEvent* event) override;

    void setNavigationStyle(View3dNavigationStyle style);

signals:
    // Emitted when a touch press is held long enough without any move(2 fingers, ...) : a context
    // menu should be displayed at position(x,y) in the view
    void touchLongPressRequested(int xPos, int yPos);

protected:
    void redrawView() override;

private:
    static Position toPosition(const QPoint& pnt) { return { pnt.x(), pnt.y() }; }
    static QPoint toQPoint(const Position& pos) { return { pos.x, pos.y }; }

    void startDynamicAction(DynamicAction action) override;
    void stopDynamicAction() override;

    void setViewCursor(const QCursor& cursor);

    std::unique_ptr<IRubberBand> createRubberBand() override;
    struct RubberBand;

    void handleEvent(const QEvent* event);
    void handleKeyPress(const QKeyEvent* event);
    void handleKeyRelease(const QKeyEvent* event);
    void handleMouseButtonPress(const QMouseEvent* event);
    void handleMouseMove(const QMouseEvent* event);
    void handleMouseButtonRelease(const QMouseEvent* event);
    void handleMouseWheel(const QWheelEvent* event);
    void handleTouch(const QTouchEvent* event);

    // -- Action matching

    // User input: key, mouse button, ...
    using Input = int;

    // Sequence of user inputs being "on" : key pressed, mouse button pressed, ...
    class InputSequence {
    public:
        void push(Input in);
        void release(Input in);
        void clear();
        gsl::span<const Input> data() const { return m_inputs; }

        enum class Operation { None, Push, Release };
        Operation lastOperation() const { return m_lastOperation; }
        Input lastInput() const { return m_lastInput; }

        bool equal(std::initializer_list<Input> other) const;

        void setPrePushCallback(std::function<void(Input)> fn) { m_fnPrePushCallback = std::move(fn); }
        void setPreReleaseCallback(std::function<void(Input)> fn) { m_fnPreReleaseCallback = std::move(fn); }
        void setClearCallback(std::function<void()> fn) { m_fnClearCallback = std::move(fn); }

    private:
        std::vector<Input> m_inputs;
        Operation m_lastOperation = Operation::None;
        Input m_lastInput = -1;
        std::function<void(Input)> m_fnPrePushCallback;
        std::function<void(Input)> m_fnPreReleaseCallback;
        std::function<void()> m_fnClearCallback;
    };

    // Base class to provide matching of DynamicAction from an InputSequence object
    class ActionMatcher {
    public:
        explicit ActionMatcher(const InputSequence& seq) : inputs(seq) {}
        virtual ~ActionMatcher() = default;

        virtual bool matchRotation() const = 0;
        virtual bool matchPan() const = 0;
        virtual bool matchZoom() const = 0;
        virtual bool matchWindowZoom() const = 0;

        virtual void onInputPrePush(Input) {}
        virtual void onInputPreRelease(Input) {}
        virtual void onInputCleared() {}

        const InputSequence& inputs;
    };

    // Fabrication to create corresponding ActionMatcher from navigation style
    static std::unique_ptr<ActionMatcher> createActionMatcher(
        View3dNavigationStyle style, const InputSequence& seq
    );
    class Mayo_ActionMatcher;
    class Catia_ActionMatcher;
    class SolidWorks_ActionMatcher;
    class Unigraphics_ActionMatcher;
    class ProEngineer_ActionMatcher;

    // -- Touch screen input

    // State of the ongoing touch gesture(2 fingers pinch to zoom, 1 finger rotation, tap selection)
    struct TouchGesture {
        bool isActive = false;
        bool isTwoFinger = false;
        bool hasMoved = false;
        QPointF pressPos;
        QPointF prevCentroid;
        double prevSpread = 0.;
        QElapsedTimer timer;
    };

    // -- Attributes

    IWidgetOccView* m_occView = nullptr;
    Position m_prevPos;
    View3dNavigationStyle m_navigStyle = View3dNavigationStyle::Mayo;
    InputSequence m_inputSequence;
    std::unique_ptr<ActionMatcher> m_actionMatcher;
    TouchGesture m_touchGesture;
    QTimer* m_touchLongPressTimer = nullptr;
    bool m_touchSession = false;
};

} // namespace Mayo
