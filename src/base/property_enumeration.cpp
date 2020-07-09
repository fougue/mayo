/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_enumeration.h"
#include <QtCore/QVariant>

namespace Mayo {

void Enumeration::addItem(Value value, const QString& name)
{
    const Item item = { value, name};
    m_vecItem.emplace_back(std::move(item));
}

int Enumeration::size() const
{
    return int(m_vecItem.size());
}

int Enumeration::findIndex(Value value) const
{
    auto it = std::find_if(
                m_vecItem.cbegin(),
                m_vecItem.cend(),
                [=](const Item& item) { return item.value == value; });
    return it != m_vecItem.cend() ? it - m_vecItem.cbegin() : -1;
}

Enumeration::Value Enumeration::findValue(const QString& name) const
{
    for (const Item& item : m_vecItem) {
        if (item.name == name)
            return item.value;
    }

    Q_ASSERT(false);
    return -1;
}

QString Enumeration::findName(Value value) const
{
    const int index = this->findIndex(value);
    return index != -1 ? this->itemAt(index).name : QString();
}

Enumeration::Item Enumeration::itemAt(int index) const
{
    return m_vecItem.at(index);
}

Span<const Enumeration::Item> Enumeration::items() const
{
    return m_vecItem;
}

PropertyEnumeration::PropertyEnumeration(
        PropertyOwner* owner,
        const QString& label,
        const Enumeration* enumeration)
    : Property(owner, label),
      m_enumeration(enumeration)
{
    Q_ASSERT(m_enumeration != nullptr);
    Q_ASSERT(m_enumeration->size() > 0);
    m_value = m_enumeration->itemAt(0).value;
}

const Enumeration& PropertyEnumeration::enumeration() const
{
    return *m_enumeration;
}

QString PropertyEnumeration::name() const
{
    return m_enumeration->findName(m_value);
}

Enumeration::Value PropertyEnumeration::value() const
{
    return m_value;
}

Result<void> PropertyEnumeration::setValue(Enumeration::Value v)
{
    // TODO: check v is an enumerated value of m_enumeration
    return Property::setValueHelper(this, &m_value, v);
}

QVariant PropertyEnumeration::valueAsVariant() const
{
    return QVariant::fromValue(m_value);
}

Result<void> PropertyEnumeration::setValueFromVariant(const QVariant& value)
{
    return this->setValue(value.toInt());
}

const char* PropertyEnumeration::dynTypeName() const
{
    return PropertyEnumeration::TypeName;
}

const char PropertyEnumeration::TypeName[] = "Mayo::PropertyEnumeration";

} // namespace Mayo
