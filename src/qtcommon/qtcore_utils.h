/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/global.h"
#include <QtCore/QByteArray>
#include <functional>
#include <string_view>

// Provides a collection of tools for the QtCore module
namespace Mayo::QtCoreUtils {

// Convenience function over QByteArray::fromRawData() taking a QByteArray object
QByteArray QByteArray_frowRawData(const QByteArray& bytes);

// Convenience function over QByteArray::fromRawData() taking a std::string_view object
QByteArray QByteArray_frowRawData(std::string_view str);

// Convenience function over QByteArray::fromRawData() taking a C array of characters
template<size_t N>
QByteArray QByteArray_frowRawData(const char (&str)[N])
{
    return QByteArray::fromRawData(str, N);
}

// Converts Mayo::CheckState -> Qt::CheckState
Qt::CheckState toQtCheckState(Mayo::CheckState state);

// Converts Qt::CheckState -> Mayo::CheckState
Mayo::CheckState toCheckState(Qt::CheckState state);

// Enqueues function 'fn' to be executed on main thread
void runJobOnMainThread(const std::function<void()>& fn);

} // namespace Mayo::QtCoreUtils
