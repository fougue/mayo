/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "qt_thread_dispatch.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QMetaObject>
#include <QtCore/QThread>

#include <stdexcept>

namespace Mayo {

QtThreadDispatch::QtThreadDispatch(QObject* context)
    : m_targetThread(context ? context->thread() : nullptr)
{
    if (!context)
        throw std::invalid_argument("QtThreadDispatch requires a context QObject");

    if (QThread::currentThread() != m_targetThread)
        throw std::logic_error("QtThreadDispatch must be created on the context thread");

    // Keep queued jobs on a dedicated receiver so shutdown can drain only this dispatcher's jobs
    m_receiver = new QObject(context);
}

bool QtThreadDispatch::isCurrentThread() const
{
    // m_targetThread is const and QThread::currentThread() is thread-local : no lock needed
    return m_targetAvailable.load() && QThread::currentThread() == m_targetThread;
}

bool QtThreadDispatch::post(std::function<void()> job)
{
    std::lock_guard lock(m_mutex);
    if (!job || !m_acceptingPosts || !m_receiver)
        return false;

    return QMetaObject::invokeMethod(m_receiver.data(), std::move(job), Qt::QueuedConnection);
}

void QtThreadDispatch::shutdown()
{
    if (QThread::currentThread() != m_targetThread)
        throw std::logic_error("QtThreadDispatch::shutdown() must run on the target thread");

    QObject* receiver = nullptr;
    {
        std::lock_guard lock(m_mutex);
        if (!m_targetAvailable)
            return;

        m_acceptingPosts = false; // from here, post() refuses; accepted jobs are already queued
        receiver = m_receiver.data();
    }

    // Run accepted jobs before the target event loop is stopped. This releases workers
    // synchronously waiting for their results through IThreadDispatch::invoke().
    if (receiver)
        QCoreApplication::sendPostedEvents(receiver, QEvent::MetaCall);

    {
        std::lock_guard lock(m_mutex);
        m_targetAvailable.store(false);
        m_receiver = nullptr;
    }

    // Delete on the receiver's affinity thread. The receiver is a child of `context`.
    delete receiver;
}

} // namespace Mayo
