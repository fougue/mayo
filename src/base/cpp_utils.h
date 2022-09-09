/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#ifndef __cpp_lib_integer_comparison_functions
#  include <limits>
#  include <type_traits>
#endif

namespace Mayo {

namespace CppUtils {

inline const std::string& nullString()
{
    static std::string str;
    return str;
}

template<typename KEY, typename VALUE, typename... EXTRAS>
VALUE findValue(const KEY& key, const std::unordered_map<KEY, VALUE, EXTRAS...>& hashmap) {
    auto it = hashmap.find(key);
    const VALUE defaultValue = {};
    return it != hashmap.cend() ? it->second : defaultValue;
}

inline void toggle(bool& value)
{
    value = !value;
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpEqual(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_equal(t, u);
#else
    using UT = std::make_unsigned_t<T>;
    using UU = std::make_unsigned_t<U>;
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
        return t == u;
    else if constexpr (std::is_signed_v<T>)
        return t < 0 ? false : UT(t) == u;
    else
        return u < 0 ? false : t == UU(u);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpNotEqual(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_not_equal(t, u);
#else
    return !cmpEqual(t, u);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpLess(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_less(t, u);
#else
    using UT = std::make_unsigned_t<T>;
    using UU = std::make_unsigned_t<U>;
    if constexpr (std::is_signed_v<T> == std::is_signed_v<U>)
        return t < u;
    else if constexpr (std::is_signed_v<T>)
        return t < 0 ? true : UT(t) < u;
    else
        return u < 0 ? false : t < UU(u);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpGreater(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_greater(t, u);
#else
    return cmpLess(u, t);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpLessEqual(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_less_equal(t, u);
#else
    return !cmpGreater(t, u);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/intcmp
template<typename T, typename U> constexpr bool cmpGreaterEqual(T t, U u) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::cmp_greater_equal(t, u);
#else
    return !cmpLess(t, u);
#endif
}

// Implementation from https://en.cppreference.com/w/cpp/utility/in_range
template<typename R, typename T> constexpr bool inRange(T t) noexcept
{
#ifdef __cpp_lib_integer_comparison_functions
    return std::in_range(t, u);
#else
    return cmpGreaterEqual(t, std::numeric_limits<R>::lowest())
            && cmpLessEqual(t, std::numeric_limits<R>::max());
#endif
}

// Throws object of specified error type if 'condition' is met
template<typename ERROR, typename... ERROR_ARGS> void throwErrorIf(bool condition, ERROR_ARGS... args)
{
    if (condition) {
        throw ERROR(args...);
    }
}

// Same as static_cast<R>(t) but throw exception if 't' does not fit inside type 'R'
template<typename R, typename T> constexpr R safeStaticCast(T t)
{
    throwErrorIf<std::overflow_error>(!inRange<R>(t), "Value too big to fit inside range type");
    return static_cast<R>(t);
}

} // namespace CppUtils
} // namespace Mayo
