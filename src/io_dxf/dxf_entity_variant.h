/****************************************************************************
** Copyright (c) 2026, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <memory>
#include <type_traits>
#include <variant>

#include "dxf_format_common.h"

struct Dxf_3DFACE;
struct Dxf_ARC;
struct Dxf_CIRCLE;
struct Dxf_ELLIPSE;
struct Dxf_INSERT;
struct Dxf_LINE;
struct Dxf_LWPOLYLINE;
struct Dxf_MTEXT;
struct Dxf_POINT;
struct Dxf_POLYLINE;
struct Dxf_SOLID;
struct Dxf_SPLINE;
struct Dxf_TEXT;
struct Dxf_ATTRIB;

using Dxf_EntityVariant = std::variant<
    std::monostate,
    std::reference_wrapper<const Dxf_3DFACE>,
    std::reference_wrapper<const Dxf_ARC>,
    std::reference_wrapper<const Dxf_CIRCLE>,
    std::reference_wrapper<const Dxf_ELLIPSE>,
    std::reference_wrapper<const Dxf_INSERT>,
    std::reference_wrapper<const Dxf_LINE>,
    std::reference_wrapper<const Dxf_LWPOLYLINE>,
    std::reference_wrapper<const Dxf_MTEXT>,
    std::reference_wrapper<const Dxf_POINT>,
    std::reference_wrapper<const Dxf_POLYLINE>,
    std::reference_wrapper<const Dxf_SOLID>,
    std::reference_wrapper<const Dxf_SPLINE>,
    std::reference_wrapper<const Dxf_TEXT>,
    std::reference_wrapper<const Dxf_ATTRIB>
>;

// Utility function that tries to read the T held inside `variant` and returns a pointer to that
// object if it exists(nullptr otherwise)
template<typename T>
constexpr const std::decay_t<T>* dxfEntityVariantGet(const Dxf_EntityVariant& variant)
{
    using U = std::decay_t<T>; // Removes const, volatile, &, &&
    auto ptr = std::get_if<std::reference_wrapper<const U>>(&variant);
    return ptr ? std::addressof(ptr->get()) : nullptr;
}
