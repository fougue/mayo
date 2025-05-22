/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#ifndef __cpp_lib_integer_comparison_functions
#  include <limits>
#  include <type_traits>
#endif

namespace Mayo {

// Declare CppUtils namespace and its "short" alias
namespace CppUtils {}
namespace Cpp = CppUtils;

// --
// -- Declaration
// --

namespace CppUtils {

// Type alias for the value type associated(mapped) to a key for an associative container(eg std::map)
template<typename AssociativeContainer>
using MappedType = typename AssociativeContainer::mapped_type;

// Type alias for the key type of an associative container(eg std::map)
template<typename AssociativeContainer>
using KeyType = typename AssociativeContainer::key_type;

// Returns a default empty std::string object, whose memory address can be used safely
inline const std::string& nullString();

// Finds in associative container the value mapped to 'key'
// If 'key' isn't found then a default-constructed value is returned
// The value(mapped) type must default-constructible
template<typename AssociativeContainer>
MappedType<AssociativeContainer> findValue(
    const KeyType<AssociativeContainer>& key,
    const AssociativeContainer& container
);

// Toggles input boolean value(eg true->false)
inline void toggle(bool& value);

// Same as std::cmp_equal() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpEqual(T t, U u) noexcept;

// Same as std::cmp_not_equal() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpNotEqual(T t, U u) noexcept;

// Same as std::cmp_less() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpLess(T t, U u) noexcept;

// Same as std::cmp_greater() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpGreater(T t, U u) noexcept;

// Same as std::cmp_less_equal() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpLessEqual(T t, U u) noexcept;

// Same as std::cmp_greater_equal() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename T, typename U> constexpr bool cmpGreaterEqual(T t, U u) noexcept;

// Same as std::in_range() but provides an implementation if !__cpp_lib_integer_comparison_functions
template<typename R, typename T> constexpr bool inRange(T t) noexcept;

// Throws object of specified error type if 'condition' is met
template<typename ErrorType, typename... ErrorArgs>
void throwErrorIf(bool condition, ErrorArgs... args);

// Same as static_cast<R>(t) but throws exception if 't' does not fit inside type 'R'
template<typename R, typename T> constexpr R safeStaticCast(T t);

} // namespace CppUtils

// --
// -- Implementation
// --

namespace CppUtils {

const std::string& nullString()
{
    static std::string str;
    return str;
}

template<typename AssociativeContainer>
MappedType<AssociativeContainer> findValue(
        const KeyType<AssociativeContainer>& key, const AssociativeContainer& container
    )
{
    auto it = container.find(key);
    if (it != container.cend()) {
        return it->second;
    }
    else {
        const typename AssociativeContainer::mapped_type defaultValue = {};
        return defaultValue;
    }
}

void toggle(bool& value)
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

template<typename ErrorType, typename... ErrorArgs> void throwErrorIf(bool condition, ErrorArgs... args)
{
    if (condition) {
        throw ErrorType(args...);
    }
}

template<typename R, typename T> constexpr R safeStaticCast(T t)
{
    throwErrorIf<std::overflow_error>(!inRange<R>(t), "Value too big to fit inside range type");
    return static_cast<R>(t);
}

} // namespace CppUtils

} // namespace Mayo
