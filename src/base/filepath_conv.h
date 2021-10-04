/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "filepath.h"

#include <TCollection_ExtendedString.hxx>

namespace Mayo {

// Base FilePath -> X converter
template<typename T> struct FilePathConv {};

// Returns the object converted to 'T' from filepath 'fp'
template<typename T> T filepathTo(const FilePath& fp) {
    return FilePathConv<T>::to(fp);
}

// FilePath -> TCollection_ExtendedString
template<> struct FilePathConv<TCollection_ExtendedString> {
    static auto to(const FilePath& fp) {
        // Can't use "if constexpr(std::is_same_v<FilePath::value_type::char>)" here because
        // discarded statements are compiled. On Windows compilation would fail.
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        // Windows -> FilePath::value_type is "char"
        return TCollection_ExtendedString(fp.c_str());
#else
        // POSIX   -> FilePath::value_type is "wchar_t"
        return TCollection_ExtendedString(fp.c_str(), true/*multi-byte*/);
#endif
    }
};

} // namespace Mayo
