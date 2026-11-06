/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Mayo {

// Dispatches jobs to a specific thread and provides synchronous invocation on that thread
// Implementations own accepted jobs until they are invoked or discarded during shutdown
class IThreadDispatch {
public:
    virtual ~IThreadDispatch() = default;

    // Returns whether the calling thread is the thread targeted by this dispatcher
    virtual bool isCurrentThread() const = 0;

    // Returns false only if the job was not accepted and will never be invoked
    // If accepted, the job is owned by the dispatcher and will either be invoked exactly once on
    // the target thread, or destroyed without invocation during shutdown
    // Accepted jobs must not be retained indefinitely
    virtual bool post(std::function<void()> job) = 0;

    // Invokes `fn` on the target thread and returns its result
    // If called from the target thread, `fn` runs immediately; otherwise this call blocks until the
    // posted job runs or is discarded during shutdown
    // Throws std::runtime_error if the job is rejected or discarded before it runs
    // Exceptions thrown by `fn` are rethrown, except `broken_promise` errors, which become
    // std::runtime_error
    template<typename Fn>
    auto invoke(Fn&& fn) -> std::invoke_result_t<std::decay_t<Fn>&>
    {
        using Result = std::invoke_result_t<std::decay_t<Fn>&>;

        if (this->isCurrentThread())
            return fn();

        auto task = std::make_shared<std::packaged_task<Result()>>(std::forward<Fn>(fn));
        std::future<Result> future = task->get_future();

        // Job destroyed before executing => packaged_task destroyed => future_error(broken_promise)
        const bool okPost = this->post([=]{ (*task)(); });
        if (!okPost)
            throw std::runtime_error("Target thread executor is unavailable");

        try {
            // IMPORTANT
            // Caller doesn't keep any reference to the task, otherwise in case of job cancellation
            // the future would never receive broken_promise (and never ends)
            task.reset();
            return future.get();
        }
        catch (const std::future_error& err) {
            if (err.code() == std::future_errc::broken_promise)
                throw std::runtime_error("Target thread ended before the job could run");
            else
                throw; // Rethrow current exception (keeping dynamic type as is)
        }
    }
};

} // namespace Mayo
