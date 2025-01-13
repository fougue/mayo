/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qt_signal_thread_helper.h"

#include <QtCore/QTimer>

namespace Mayo {

std::any QtSignalThreadHelper::getCurrentThreadContext()
{
    // Note: thread_local implies "static"
    //       See https://en.cppreference.com/w/cpp/language/storage_duration
    thread_local QObject obj;
    return &obj;
}

void QtSignalThreadHelper::execInThread(const std::any& context, const std::function<void()>& fn)
{
    auto qobject = std::any_cast<QObject*>(context);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QTimer::singleShot(0, qobject, fn);
#else
    QMetaObject::invokeMethod(qobject, fn, Qt::ConnectionType::QueuedConnection);
#endif
}

} // namespace Mayo
