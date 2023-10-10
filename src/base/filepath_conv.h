/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "filepath.h"
#include "global.h"

#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <string_view>

namespace Mayo {

// Base FilePath -> X converter
template<typename T> struct FilePathConv {};

// Returns the object converted to 'T' from filepath 'fp'
template<typename T> T filepathTo(const FilePath& fp) {
    return FilePathConv<T>::to(fp);
}

// FilePath -> TCollection_AsciiString
template<> struct FilePathConv<TCollection_AsciiString> {
    static auto to(const FilePath& fp) {
        return TCollection_AsciiString(fp.u8string().c_str());
    }
};

// FilePath -> TCollection_ExtendedString
template<> struct FilePathConv<TCollection_ExtendedString> {
    static auto to(const FilePath& fp) {
        // Can't use "if constexpr(std::is_same_v<FilePath::value_type::char>)" here because
        // discarded statements are compiled. On Windows compilation would fail.
#ifdef MAYO_OS_WINDOWS
        // Windows -> FilePath::value_type is "char"
        return TCollection_ExtendedString(fp.c_str());
#else
        // POSIX   -> FilePath::value_type is "wchar_t"
        return TCollection_ExtendedString(fp.c_str(), true/*multi-byte*/);
#endif
    }
};

// std::string_view -> FilePath
// Assumes utf8 encoding
inline FilePath filepathFrom(std::string_view strUtf8) {
    return std_filesystem::u8path(strUtf8);
}

// TCollection_AsciiString -> FilePath
// Assumes utf8 encoding
inline FilePath filepathFrom(const TCollection_AsciiString& strUtf8) {
    return std_filesystem::u8path(strUtf8.ToCString());
}

} // namespace Mayo

