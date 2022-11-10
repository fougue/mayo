/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// 'emit' macro might have been defined by Qt as a pseudo additional keyword
// It's temporarily undefined in this header for the sake of KBindings::Signal which provides member
// function emit()
#ifdef emit
#  undef emit
#  define MAYO_QT_EMIT_TO_BE_RESTORED
#endif

#include <kdbindings/signal.h>
#include <any>
#include <memory>
#include <thread>

namespace Mayo {

using SignalConnectionHandle = KDBindings::ConnectionHandle;

// Provides an interface to deal with signal/slot thread mismatch
//
// Typical example is to connect a signal to a slot in main(UI) thread but later on the signal is
// emitted from within a worker thread. In such case slots would be executed by the worker thread
// which might cause troubles
// ISignalThreadHelper is used by Signal::connectSlot() to make sure the slots are executed in the
// thread where the initial connection was created.
class ISignalThreadHelper {
public:
    virtual ~ISignalThreadHelper() = default;

    // Get implementation-specific context object for the current thread
    virtual std::any getCurrentThreadContext() = 0;

    // Executes function 'fn' in target thread referred by 'context'
    // If current thread is different from target thread then implementation has to ensure 'fn' is
    // executed in target thread(eg by enqueuing 'fn' in some event loop)
    // 'context' is the implementation-specific thread context object identified in a previous call
    // to getCurrentThreadContext()
    virtual void execInThread(const std::any& context, const std::function<void()>& fn) = 0;
};

// Getter/setter functions of the global ISignalThreadHelper object used by Signal::connectSlot()
// ISignalThreadHelper is optional(by default getGlobalSignalThreadHelper() returns nullptr) in that
// case expect signal/slot thread mismatches(which might be fine with user client-code)
ISignalThreadHelper* getGlobalSignalThreadHelper();
void setGlobalSignalThreadHelper(std::unique_ptr<ISignalThreadHelper> helper);

// Provides a mechanism for communication between objects
// Based on KDBindings::Signal<>
//
// Prefer connectSlot() in client-code instead of KDBindings::Signal::connect() because it
// uses ISignalThreadHelper to handle signal/slot thread mismatch
// On signal emission KDBindings::Signal makes direct call to the connected slot functions which is
// cause of problems
template<typename... Args>
class Signal : public KDBindings::Signal<Args...> {
public:
    Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(Signal const &other) = delete;
    Signal(Signal&& other) noexcept = default;
    Signal& operator=(Signal&& other) noexcept = default;

    // Emits the Signal, which causes all connected slots to be called, as long as they are not
    // blocked.
    // The arguments provided to emit will be passed to each slot by copy, therefore consider
    // using (const) references as the Args to the Signal wherever possible.
    void send(Args... p) const
    {
        this->emit(p...);
    }

    // Connects 'fnSlot' to the signal
    // When send() is called, the functions will be executed with the arguments provided to send()
    SignalConnectionHandle connectSlot(const std::function<void(Args...)>& fnSlot)
    {
        if (getGlobalSignalThreadHelper()) {
            auto threadContext = getGlobalSignalThreadHelper()->getCurrentThreadContext();
            auto connectThreadId = std::this_thread::get_id();
            auto fnWrap = [=](Args... args) {
                auto emitThreadId = std::this_thread::get_id();
                if (emitThreadId == connectThreadId)
                    fnSlot(args...);
                else
                    getGlobalSignalThreadHelper()->execInThread(threadContext, [=]{ fnSlot(args...); });
            };
            return this->connect(fnWrap);
        }
        else {
            return this->connect(fnSlot);
        }
    }

    // Template overload of Signal::connectSlot() making easier arbitrary connections of functions
    // to this signal
    // It connects a function to this Signal, binds any provided arguments to that function and
    // discards any values emitted by this Signal that aren't needed by the resulting function.
    // Especially useful for connecting member functions to signals
    template<typename FunctionSlot, typename... FunctionSlotArgs, typename = std::enable_if_t<std::disjunction_v<std::negation<std::is_convertible<FunctionSlot, std::function<void(Args...)>>>, std::integral_constant<bool, sizeof...(FunctionSlotArgs) /*Also enable this function if we want to bind at least one argument*/>>>>
    SignalConnectionHandle connectSlot(FunctionSlot&& fnSlot, FunctionSlotArgs&&... args)
    {
        std::function<void(Args...)> bound = KDBindings::Private::bind_first(std::forward<FunctionSlot>(fnSlot), std::forward<FunctionSlotArgs>(args)...);
        return this->connectSlot(bound);
    }
};

} // namespace Mayo

// Restore 'emit' Qt macro if it was undefined at the beginning of this header
#ifdef MAYO_QT_EMIT_TO_BE_RESTORED
#  define emit
#endif
#undef MAYO_QT_EMIT_TO_BE_RESTORED
