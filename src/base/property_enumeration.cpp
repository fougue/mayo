/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_enumeration.h"

namespace Mayo {

PropertyEnumeration::PropertyEnumeration(
        PropertyGroup* grp, const TextId& name, const Enumeration& enumeration)
    : Property(grp, name),
      m_value(!enumeration.empty() ? enumeration.items().front().value : -1),
      m_enumeration(enumeration)
{
}

void PropertyEnumeration::addDescription(Enumeration::Value value, std::string_view descr)
{
    m_vecDescription.push_back({ value, std::string(descr) });
}

std::string_view PropertyEnumeration::findDescription(Enumeration::Value value) const
{
    auto itFound = std::find_if(
                m_vecDescription.cbegin(), m_vecDescription.cend(), [=](const Description& descr) {
       return descr.value == value;
    });
    return itFound != m_vecDescription.cend() ? itFound->text : std::string_view{};
}

void PropertyEnumeration::clearDescriptions()
{
    m_vecDescription.clear();
}

std::string_view PropertyEnumeration::name() const
{
    return m_enumeration.findNameByValue(m_value);
}

Enumeration::Value PropertyEnumeration::value() const
{
    return m_value;
}

bool PropertyEnumeration::setValue(Enumeration::Value value)
{
    // TODO: check v is an enumerated value of m_enumeration
    return Property::setValueHelper(this, &m_value, value);
}

bool PropertyEnumeration::setValueByName(std::string_view name)
{
    return Property::setValueHelper(this, &m_value, m_enumeration.findValueByName(name));
}

const char* PropertyEnumeration::dynTypeName() const
{
    return PropertyEnumeration::TypeName;
}

const char PropertyEnumeration::TypeName[] = "Mayo::PropertyEnumeration";

} // namespace Mayo
