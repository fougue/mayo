/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/global.h"

#include <QtCore/QByteArray>
#include <gsl/span>
#include <string_view>
#include <vector>

// Provides a collection of tools for the QtCore module
namespace Mayo::QtCoreUtils {

// Convenience function over QByteArray::fromRawData() taking a QByteArray object
QByteArray QByteArray_fromRawData(const QByteArray& bytes);

// Convenience function over QByteArray::fromRawData() taking a std::string_view object
QByteArray QByteArray_fromRawData(std::string_view str);

// Convenience function over QByteArray::fromRawData() taking a C array of characters
template<size_t N>
QByteArray QByteArray_fromRawData(const char (&str)[N])
{
    return QByteArray_fromRawData(std::string_view{str, N});
}

// Convenience function over QByteArray::fromRawData() taking a span of bytes
template<typename ByteType>
QByteArray QByteArray_fromRawData(gsl::span<const ByteType> bytes)
{
    static_assert(sizeof(ByteType) == 1, "size of ByteType must be one byte");
    return QByteArray_fromRawData(
        std::string_view{ reinterpret_cast<const char*>(bytes.data()), bytes.size() }
    );
}

// Constructs a byte array containing the first 'size' bytes of array 'data'
QByteArray toQByteArray(const char* data, size_t size);

// Converts a span of bytes to QByteArray object
template<typename ByteType>
QByteArray toQByteArray(gsl::span<const ByteType> bytes)
{
    static_assert(sizeof(ByteType) == 1, "size of ByteType must be one byte");
    return toQByteArray(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

// Converts a QByteArray object to std::vector<uint8_t>
std::vector<uint8_t> toStdByteArray(const QByteArray& bytes);

// Converts Mayo::CheckState -> Qt::CheckState
Qt::CheckState toQtCheckState(Mayo::CheckState state);

// Converts Qt::CheckState -> Mayo::CheckState
Mayo::CheckState toCheckState(Qt::CheckState state);

} // namespace Mayo::QtCoreUtils
