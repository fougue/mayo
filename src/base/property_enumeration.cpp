/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_enumeration.h"

#include <algorithm>
#include <stdexcept>

namespace Mayo {

PropertyEnumeration::PropertyEnumeration(
        PropertyGroup* grp, const TextId& name, const Enumeration* enumeration
    )
    : Property(grp, name),
      m_enumeration(enumeration),
      m_value(enumeration && !enumeration->empty() ? enumeration->itemAt(0).value : -1)
{
}

const Enumeration& PropertyEnumeration::enumeration() const
{
    if (!m_enumeration)
        throw std::runtime_error("Internal Enumeration object must not be null");

    return *m_enumeration;
}

PropertyEnumeration::PropertyEnumeration(PropertyGroup* grp, const TextId& name)
    : Property(grp, name)
{
}

void PropertyEnumeration::setEnumeration(const Enumeration* enumeration)
{
    m_enumeration = enumeration;
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

std::string_view PropertyEnumeration::valueName() const
{
    return m_enumeration ? m_enumeration->findNameByValue(m_value) : std::string_view{};
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
    if (!m_enumeration)
        return false;

    return Property::setValueHelper(this, &m_value, m_enumeration->findValueByName(name));
}

const char* PropertyEnumeration::dynTypeName() const
{
    return PropertyEnumeration::TypeName;
}

const char PropertyEnumeration::TypeName[] = "Mayo::PropertyEnumeration";

} // namespace Mayo
