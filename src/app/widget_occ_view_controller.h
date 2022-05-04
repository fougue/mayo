/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../graphics/v3d_view_controller.h"
#include "../base/span.h"

#include <functional>
#include <memory>
#include <vector>
class QCursor;
class QKeyEvent;
class QMouseEvent;
class QRubberBand;
class QWheelEvent;

namespace Mayo {

class IWidgetOccView;

class WidgetOccViewController : public V3dViewController {
    Q_OBJECT
public:
    WidgetOccViewController(IWidgetOccView* occView = nullptr);

    bool eventFilter(QObject* watched, QEvent* event) override;

    enum class NavigationStyle {
        Mayo/*Default*/, Catia, SolidWorks, Unigraphics, ProEngineer
    };
    void setNavigationStyle(NavigationStyle style);

signals:
    void multiSelectionToggled(bool on);

protected:
    void redrawView() override;

private:
    void startDynamicAction(DynamicAction action) override;
    void stopDynamicAction() override;

    void setViewCursor(const QCursor& cursor);

    AbstractRubberBand* createRubberBand() override;
    struct RubberBand;

    void handleEvent(const QEvent* event);
    void handleKeyPress(const QKeyEvent* event);
    void handleKeyRelease(const QKeyEvent* event);
    void handleMouseButtonPress(const QMouseEvent* event);
    void handleMouseMove(const QMouseEvent* event);
    void handleMouseButtonRelease(const QMouseEvent* event);
    void handleMouseWheel(const QWheelEvent* event);

    // -- Input matching

    using Input = int;
    class InputSequence {
    public:
        void push(Input in);
        void release(Input in);
        void clear();
        Span<const Input> data() const { return m_inputs; }

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

    class InputMatcher {
    public:
        InputMatcher(const InputSequence* seq) : m_seq(*seq) {}
        virtual ~InputMatcher() = default;

        virtual bool matchRotation() const = 0;
        virtual bool matchPan() const = 0;
        virtual bool matchZoom() const = 0;
        virtual bool matchWindowZoom() const = 0;

        virtual void onInputPrePush(Input) {}
        virtual void onInputPreRelease(Input) {}
        virtual void onInputCleared() {}

    protected:
        const InputSequence& inputs() const { return m_seq; }

    private:
        const InputSequence& m_seq;
    };
    static std::unique_ptr<InputMatcher> createInputMatcher(NavigationStyle style, const InputSequence* seq);
    class Mayo_InputMatcher;
    class Catia_InputMatcher;
    class SolidWorks_InputMatcher;
    class Unigraphics_InputMatcher;
    class ProEngineer_InputMatcher;

    // -- Attributes

    IWidgetOccView* m_occView = nullptr;
    QPoint m_prevPos;
    NavigationStyle m_navigStyle = NavigationStyle::Mayo;
    InputSequence m_inputSequence;
    std::unique_ptr<InputMatcher> m_inputMatcher;
};

} // namespace Mayo
