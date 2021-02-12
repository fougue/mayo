/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_enumeration.h"
#include <QtCore/QVariant>

namespace Mayo {

PropertyEnumeration::PropertyEnumeration(
        PropertyGroup* grp, const TextId& name, const Enumeration& enumeration)
    : Property(grp, name),
      m_enumeration(enumeration)
{
}

void PropertyEnumeration::addDescription(Enumeration::Value value, const QString& descr)
{
    m_vecDescription.push_back({ value, descr });
}

QString PropertyEnumeration::findDescription(Enumeration::Value value) const
{
    auto itFound = std::find_if(
                m_vecDescription.cbegin(), m_vecDescription.cend(), [=](const Description& descr) {
       return descr.value == value;
    });
    return itFound != m_vecDescription.cend() ? itFound->text: QString();
}

void PropertyEnumeration::clearDescriptions()
{
    m_vecDescription.clear();
}

QByteArray PropertyEnumeration::name() const
{
    return m_enumeration.findName(m_value);
}

Enumeration::Value PropertyEnumeration::value() const
{
    return m_value;
}

Result<void> PropertyEnumeration::setValue(Enumeration::Value value)
{
    // TODO: check v is an enumerated value of m_enumeration
    return Property::setValueHelper(this, &m_value, value);
}

Result<void> PropertyEnumeration::setValueByName(const QByteArray& name)
{
    return Property::setValueHelper(this, &m_value, m_enumeration.findValue(name));
}

QVariant PropertyEnumeration::valueAsVariant() const
{
    return QVariant::fromValue(this->name());
}

Result<void> PropertyEnumeration::setValueFromVariant(const QVariant& value)
{
    const QByteArray name = value.toByteArray();
    const Enumeration::Item* ptrItem = m_enumeration.findItem(name);
    if (ptrItem)
        return this->setValue(ptrItem->value);

    return Result<void>::error();
}

const char* PropertyEnumeration::dynTypeName() const
{
    return PropertyEnumeration::TypeName;
}

const char PropertyEnumeration::TypeName[] = "Mayo::PropertyEnumeration";

} // namespace Mayo
