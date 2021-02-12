/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <string_view>

namespace Mayo {
namespace QtCoreUtils {

inline QByteArray QByteArray_frowRawData(const QByteArray& bytes) {
    return QByteArray::fromRawData(bytes.data(), bytes.size());
}

inline QByteArray QByteArray_frowRawData(std::string_view str) {
    return QByteArray::fromRawData(str.data(), int(str.size()));
}

template<size_t N>
QByteArray QByteArray_frowRawData(const char (&str)[N]) {
    return QByteArray::fromRawData(str, N);
}

} // namespace QtCoreUtils
} // namespace Mayo
