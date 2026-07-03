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
struct EnumNames {
    static inline const std::string_view trContext = {};
    static inline const std::string_view junkPrefix = {};
};

template<typename EnumType>
Enumeration Enumeration::fromType()
{
    const bool hasJunkPrefix = !EnumNames<EnumType>::junkPrefix.empty();
    Enumeration enumObject;
    for (const EnumType value : MetaEnum::values<EnumType>()) {
        std::string_view key =
                hasJunkPrefix ?
                    MetaEnum::nameWithoutPrefix<EnumType>(value, EnumNames<EnumType>::junkPrefix) :
                    MetaEnum::name<EnumType>(value);
        const TextId keyTextId = { EnumNames<EnumType>::trContext, key };
        enumObject.addItem(int(value), keyTextId);
    }

    return enumObject;
}

} // namespace Mayo
