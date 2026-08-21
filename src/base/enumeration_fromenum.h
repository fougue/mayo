/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "enumeration.h"
#include "meta_enum.h"

// --
// -- Implementation of template function Enumeration::fromType()
// --

namespace Mayo {

template<typename EnumType>
Enumeration Enumeration::fromType(std::string_view trContext, std::string_view junkPrefix)
{
    const bool hasJunkPrefix = !junkPrefix.empty();
    Enumeration enumObject;
    for (const EnumType value : MetaEnum::values<EnumType>()) {
        std::string_view key =
            hasJunkPrefix
                ? MetaEnum::nameWithoutPrefix<EnumType>(value, junkPrefix)
                : MetaEnum::name<EnumType>(value)
            ;
        const TextId keyTextId = { trContext, key };
        enumObject.addItem(static_cast<int>(value), keyTextId);
    }

    return enumObject;
}

} // namespace Mayo
