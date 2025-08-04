/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <gsl/span>
#include <cassert>
#include <climits>

namespace Mayo {

template<typename T> using Span = gsl::span<T>;

// Returns the index of 'item' contained in 'span'
template<typename T>
constexpr int Span_itemIndex(Span<T> span, typename Span<T>::const_reference item)
{
    assert(!span.empty());
    auto index = &item - &span.front();
    assert(index >= 0);
    assert(index <= INT_MAX);
    assert(&span[static_cast<typename Span<T>::size_type>(index)] == &item);
    return static_cast<int>(index);
}

template<typename Container>
constexpr int Span_itemIndex(const Container& cont, typename Container::const_reference item)
{
    return Span_itemIndex(Span<const typename Container::value_type>(cont), item);
}

} // namespace Mayo
