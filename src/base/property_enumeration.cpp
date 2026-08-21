/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "property_enumeration.h"

#include <algorithm>
#include <stdexcept>

namespace Mayo {

PropertyEnum::PropertyEnum(PropertyGroup* grp, const PropertyEnumMeta& meta)
    : Property(grp, meta),
      m_value(!meta.enumeration().empty() ? meta.enumeration().itemAt(0).value : -1)
{
}

const PropertyEnumMeta& PropertyEnum::enumMeta() const
{
    return static_cast<const PropertyEnumMeta&>(this->meta());
}

TextId PropertyEnumMeta::findDescription(Enumeration::Value value) const
{
    const Enumeration::Item* ptrItem = m_enum.findItemByValue(value);
    return ptrItem ? ptrItem->description : TextId{};
}

TextId PropertyEnum::valueName() const
{
    return this->enumMeta().enumeration().findNameByValue(m_value);
}

Enumeration::Value PropertyEnum::value() const
{
    return m_value;
}

bool PropertyEnum::setValue(Enumeration::Value value)
{
    // TODO: check v is an enumerated value of m_enumeration
    return Property::setValueHelper(this, &m_value, value);
}

bool PropertyEnum::setValueByName(std::string_view name)
{
    const auto enumValue = this->enumMeta().enumeration().findValueByName(name);
    return Property::setValueHelper(this, &m_value, enumValue);
}

bool PropertyEnum::copyValue(const Property& other)
{
    if (this->dynTypeName() == other.dynTypeName()) {
        const auto& otherPropEnum = static_cast<const PropertyEnum&>(other);
        if (&this->enumMeta().enumeration() == &otherPropEnum.enumMeta().enumeration())
            return this->setValue(otherPropEnum.value());
    }

    return false;
}

const char* PropertyEnum::dynTypeName() const
{
    return PropertyEnum::TypeName;
}

const char PropertyEnum::TypeName[] = "Mayo::PropertyEnum";

} // namespace Mayo
