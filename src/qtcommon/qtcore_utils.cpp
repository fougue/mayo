/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "qtcore_utils.h"

#include <algorithm>

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

namespace Mayo::QtCoreUtils {

QByteArray QByteArray_fromRawData(const QByteArray& bytes)
{
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

QByteArray QByteArray_fromRawData(std::string_view str)
{
    return QByteArray::fromRawData(str.data(), int(str.size()));
}

std::vector<uint8_t> toStdByteArray(const QByteArray& bytes)
{
    std::vector<uint8_t> stdBytes;
    stdBytes.resize(bytes.size());
    std::copy(bytes.cbegin(), bytes.cend(), stdBytes.begin());
    return stdBytes;
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
