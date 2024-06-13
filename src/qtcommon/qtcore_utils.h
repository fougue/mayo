/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/global.h"
#include <QtCore/QByteArray>
#include <string_view>

namespace Mayo {

// Provides a collection of tools for the QtCore module
namespace QtCoreUtils {

// Convenience function over QByteArray::fromRawData() taking a QByteArray object
inline QByteArray QByteArray_frowRawData(const QByteArray& bytes)
{
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

// Convenience function over QByteArray::fromRawData() taking a std::string_view object
inline QByteArray QByteArray_frowRawData(std::string_view str)
{
    return QByteArray::fromRawData(str.data(), int(str.size()));
}

// Convenience function over QByteArray::fromRawData() taking a C array of characters
template<size_t N>
QByteArray QByteArray_frowRawData(const char (&str)[N])
{
    return QByteArray::fromRawData(str, N);
}

// Converts Mayo::CheckState -> Qt::CheckState
inline Qt::CheckState toQtCheckState(Mayo::CheckState state)
{
    switch (state) {
    case CheckState::Off: return Qt::Unchecked;
    case CheckState::Partially: return Qt::PartiallyChecked;
    case CheckState::On: return Qt::Checked;
    }

    return Qt::Unchecked;
}

// Converts Qt::CheckState -> Mayo::CheckState
inline Mayo::CheckState toCheckState(Qt::CheckState state)
{
    switch (state) {
    case Qt::Unchecked: return CheckState::Off;
    case Qt::PartiallyChecked: return CheckState::Partially;
    case Qt::Checked: return CheckState::On;
    }

    return CheckState::Off;
}

} // namespace QtCoreUtils
} // namespace Mayo
