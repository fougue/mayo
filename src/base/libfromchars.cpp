/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "libfromchars.h"

#include <fast_float/fast_float.h>

#include <climits>
#include <cstdint>
#include <type_traits>

namespace Mayo {

namespace {

template<typename UC>
FromCharsResult ffResult(const fast_float::from_chars_result_t<UC>& ffr)
{
    return { ffr.ptr, ffr.ec };
}

} // namespace

template<typename T>
FromCharsResult fromChars(const char* first, const char* last, T& value, int base)
{
    static_assert(std::is_integral_v<T>, "Mayo::fromChars() requires an arithmetic type");
    return ffResult(fast_float::from_chars(first, last, value, base));
}

FromCharsResult fromChars(const char* first, const char* last, float& value)
{
    return ffResult(fast_float::from_chars(first, last, value));
}

FromCharsResult fromChars(const char* first, const char* last, double& value)
{
    return ffResult(fast_float::from_chars(first, last, value));
}

FromCharsResult fromChars(std::string_view str, float& value)
{
    return fromChars(str.data(), str.data() + str.size(), value);
}

FromCharsResult fromChars(std::string_view str, double& value)
{
    return fromChars(str.data(), str.data() + str.size(), value);
}

// Explicit template instanciations

template FromCharsResult fromChars(const char*, const char*, signed char&, int);
template FromCharsResult fromChars(const char*, const char*, short&, int);
template FromCharsResult fromChars(const char*, const char*, int&, int);
template FromCharsResult fromChars(const char*, const char*, long&, int);
template FromCharsResult fromChars(const char*, const char*, long long&, int);

template FromCharsResult fromChars(const char*, const char*, unsigned char&, int);
template FromCharsResult fromChars(const char*, const char*, unsigned short&, int);
template FromCharsResult fromChars(const char*, const char*, unsigned int&, int);
template FromCharsResult fromChars(const char*, const char*, unsigned long&, int);
template FromCharsResult fromChars(const char*, const char*, unsigned long long&, int);

#if defined(INT8_MAX) && (INT8_MAX != SCHAR_MAX)
template FromCharsResult fromChars(const char*, const char*, int8_t&, int);
#endif

#if defined(INT16_MAX) && (INT16_MAX != SHRT_MAX)
template FromCharsResult fromChars(const char*, const char*, int16_t&, int);
#endif

#if defined(INT32_MAX) && !(INT32_MAX == INT_MAX || INT32_MAX == LONG_MAX)
template FromCharsResult fromChars(const char*, const char*, int32_t&, int);
#endif

#if defined(INT64_MAX) && !(INT64_MAX == LONG_MAX || INT64_MAX == LLONG_MAX)
template FromCharsResult fromChars(const char*, const char*, int64_t&, int);
#endif


#if defined(UINT8_MAX) && (UINT8_MAX != UCHAR_MAX)
template FromCharsResult fromChars(const char*, const char*, uint8_t&, int);
#endif

#if defined(UINT16_MAX) && (UINT16_MAX != USHRT_MAX)
template FromCharsResult fromChars(const char*, const char*, uint16_t&, int);
#endif

#if defined(UINT32_MAX) && !(UINT32_MAX == UINT_MAX || UINT32_MAX == ULONG_MAX)
template FromCharsResult fromChars(const char*, const char*, uint32_t&, int);
#endif

#if defined(UINT64_MAX) && !(UINT64_MAX == ULONG_MAX || UINT64_MAX == ULLONG_MAX)
template FromCharsResult fromChars(const char*, const char*, uint64_t&, int);
#endif

} // namespace Mayo
