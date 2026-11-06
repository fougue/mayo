/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/ithread_dispatch.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
class QThread;

#include <atomic>
#include <functional>
#include <mutex>

namespace Mayo {

class QtThreadDispatch : public IThreadDispatch {
public:
    // Construct on the context's thread. The context must remain alive, and must not change
    // thread affinity, until shutdown() has completed
    explicit QtThreadDispatch(QObject* context);

    // Returns false once shutdown() has been called, even on the target thread
    // Consequence: invoke() called on the target thread after shutdown() is refused by post()
    // and throws, instead of calling the function directly. Code running during shutdown must
    // not assume invoke() works
    bool isCurrentThread() const override;

    // Jobs must not throw: an exception escaping into the Qt event loop is fatal
    bool post(std::function<void()> job) override;

    // Stop accepting jobs and run all accepted jobs still queued on the target thread
    // Must be called on the target thread before its event loop stops or the context is destroyed,
    // and BEFORE waiting for (or joining) any worker that may be blocked in invoke():
    // otherwise those workers wait for jobs that the waiting thread can no longer run (deadlock)
    void shutdown();

private:
    QPointer<QObject> m_receiver; // Protected by m_mutex
    QThread* const m_targetThread = nullptr;
    mutable std::mutex m_mutex;
    bool m_acceptingPosts = true; // Protected by m_mutex
    std::atomic<bool> m_targetAvailable{true};  // written under m_mutex, read lock-free
};

} // namespace Mayo
