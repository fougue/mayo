/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <magic_enum/magic_enum.hpp>
#include <string>
#include <string_view>

namespace Mayo {

// Provides meta-data helper functions about enumerated types
// Currently it wraps magic_enum 3rdparty library
class MetaEnum {
public:
    template<typename EnumType>
    static std::string_view name(EnumType enumValue) {
        return magic_enum::enum_name(enumValue);
    }

    template<typename EnumType>
    static int count() {
        return magic_enum::enum_count<EnumType>();
    }

    template<typename EnumType>
    static std::string_view nameWithoutPrefix(EnumType enumValue, std::string_view strPrefix) {
        std::string_view strEnumValueName = MetaEnum::name(enumValue);
        if (strEnumValueName.find(strPrefix) == 0)
            return strEnumValueName.substr(strPrefix.size());
        else
            return strEnumValueName;
    }

    // Returns std::array with pairs(value, name), sorted by enum value
    template<typename EnumType>
    static auto entries() {
        return magic_enum::enum_entries<EnumType>();
    }

    template<typename EnumType>
    static auto values() {
        return magic_enum::enum_values<EnumType>();
    }
};

} // namespace Mayo
