/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <cstdint>
#include <string_view>
#include <system_error>

// Mayo uses fast_float 3rd-party library as a replacement for std::from_chars()
// That library is provided as a single header-only file which is rather big
// Including fast_float.h in translation units(TU) would lead to increasing compilation times
// This file provides thin wrapping functions over fast_float::from_chars() so that the library is
// built into a single TU

namespace Mayo {

// Same as std::from_chars_result
struct FromCharsResult {
    const char* ptr;
    std::errc ec;
};

// Wrapper function for fast_float::from_chars() converting to integer values
template<typename T>
FromCharsResult fromChars(const char* first, const char* last, T& value, int base = 10);

// Overload for std::string_view
template<typename T>
FromCharsResult fromChars(std::string_view str, T& value, int base = 10) {
    return fromChars(str.data(), str.data() + str.size(), value, base);
}

// Wrapper function for fast_float::from_chars() converting to float/double values
FromCharsResult fromChars(const char* first, const char* last, float& value);
FromCharsResult fromChars(const char* first, const char* last, double& value);

// Overloads for std::string_view
FromCharsResult fromChars(std::string_view str, float& value);
FromCharsResult fromChars(std::string_view str, double& value);



// Extern templates: forbids implicit instanciations in translation units

extern template FromCharsResult fromChars(const char*, const char*, signed char&, int);
extern template FromCharsResult fromChars(const char*, const char*, short&, int);
extern template FromCharsResult fromChars(const char*, const char*, int&, int);
extern template FromCharsResult fromChars(const char*, const char*, long&, int);
extern template FromCharsResult fromChars(const char*, const char*, long long&, int);
extern template FromCharsResult fromChars(const char*, const char*, unsigned char&, int);
extern template FromCharsResult fromChars(const char*, const char*, unsigned short&, int);
extern template FromCharsResult fromChars(const char*, const char*, unsigned int&, int);
extern template FromCharsResult fromChars(const char*, const char*, unsigned long&, int);
extern template FromCharsResult fromChars(const char*, const char*, unsigned long long&, int);

extern template FromCharsResult fromChars(const char*, const char*, int8_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int16_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int32_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int64_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int8_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int16_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int32_t&, int);
extern template FromCharsResult fromChars(const char*, const char*, int64_t&, int);

} // namespace Mayo
