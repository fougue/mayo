/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qtcore_utils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

namespace Mayo::QtCoreUtils {

QByteArray QByteArray_frowRawData(const QByteArray& bytes)
{
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

QByteArray QByteArray_frowRawData(std::string_view str)
{
    return QByteArray::fromRawData(str.data(), int(str.size()));
}

Qt::CheckState toQtCheckState(Mayo::CheckState state)
{
    switch (state) {
    case CheckState::Off: return Qt::Unchecked;
    case CheckState::Partially: return Qt::PartiallyChecked;
    case CheckState::On: return Qt::Checked;
    }

    return Qt::Unchecked;
}

// Converts Qt::CheckState -> Mayo::CheckState
Mayo::CheckState toCheckState(Qt::CheckState state)
{
    switch (state) {
    case Qt::Unchecked: return CheckState::Off;
    case Qt::PartiallyChecked: return CheckState::Partially;
    case Qt::Checked: return CheckState::On;
    }

    return CheckState::Off;
}

void runJobOnMainThread(const std::function<void()>& fn)
{
    QTimer::singleShot(0, QCoreApplication::instance(), fn);
}

} // namespace Mayo::QtCoreUtils
