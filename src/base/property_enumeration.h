/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "enumeration.h"
#include "enumeration_fromenum.h"
#include "property.h"

#include <string_view>
#include <type_traits>

namespace Mayo {

class PropertyEnumMeta : public PropertyMeta {
public:
    // ⚠ `enumeration` is non-owning (keeps the reference)
    PropertyEnumMeta(const TextId& name, const Enumeration& enumeration)
        : PropertyMeta(name),
          m_enum(enumeration)
    {}

    const Enumeration& enumeration() const { return m_enum; }

    TextId findDescription(Enumeration::Value value) const;

private:
    const Enumeration& m_enum; // Non owning
};

class PropertyEnum : public Property {
public:
    using ValueType = Enumeration::Value;
    using MetaType = PropertyEnumMeta;

    PropertyEnum(PropertyGroup* grp, const PropertyEnumMeta& meta);

    const PropertyEnumMeta& enumMeta() const;
    const Enumeration& enumeration() const { return this->enumMeta().enumeration(); }

    TextId valueName() const;
    Enumeration::Value value() const;
    bool setValue(Enumeration::Value value);
    bool setValueByName(std::string_view name);

    bool copyValue(const Property& other) override;

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    Enumeration::Value m_value = -1;
};

} // namespace Mayo
