/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property_enumeration.h"
#include <QtCore/QVariant>

namespace Mayo {

namespace {

const Enumeration::Item* findEnumerationItem(const Enumeration& enumeration, const QByteArray& name)
{
    Span<const Enumeration::Item> spanItems = enumeration.items();
    auto itFound = std::find_if(
                spanItems.cbegin(),
                spanItems.cend(),
                [&](const Enumeration::Item& enumItem) { return name == enumItem.name.key; });
    return itFound != spanItems.cend() ? &(*itFound) : nullptr;
}

} // namespace

Enumeration::Enumeration(std::initializer_list<Item> listItem)
    : m_vecItem(listItem)
{
}

void Enumeration::addItem(Value value, const TextId& name)
{
    const Item item = { value, name };
    m_vecItem.emplace_back(std::move(item));
}

int Enumeration::size() const
{
    return int(m_vecItem.size());
}

const Enumeration::Item& Enumeration::findItem(Enumeration::Value value) const
{
    const int index = this->findIndex(value);
    Expects(index != -1);
    return this->itemAt(index);
}

int Enumeration::findIndex(Value value) const
{
    auto it = std::find_if(
                m_vecItem.cbegin(),
                m_vecItem.cend(),
                [=](const Item& item) { return item.value == value; });
    return it != m_vecItem.cend() ? it - m_vecItem.cbegin() : -1;
}

Enumeration::Value Enumeration::findValue(const QByteArray& name) const
{
    const Enumeration::Item* ptrItem = findEnumerationItem(*this, name);
    Q_ASSERT(ptrItem != nullptr);
    return ptrItem ? ptrItem->value : -1;
}

bool Enumeration::contains(const QByteArray& name) const
{
    return findEnumerationItem(*this, name) != nullptr;
}

QByteArray Enumeration::findName(Value value) const
{
    const int index = this->findIndex(value);
    if (index != -1)
        return this->itemAt(index).name.key;

    return QByteArray();
}

const Enumeration::Item& Enumeration::itemAt(int index) const
{
    return m_vecItem.at(index);
}

Span<const Enumeration::Item> Enumeration::items() const
{
    return m_vecItem;
}

PropertyEnumeration::PropertyEnumeration(
        PropertyGroup* grp, const TextId& name, const Enumeration* enumeration)
    : Property(grp, name)
{
    this->setEnumeration(enumeration);
}

const Enumeration* PropertyEnumeration::enumeration() const
{
    return m_enumeration;
}

void PropertyEnumeration::setEnumeration(const Enumeration* enumeration)
{
    m_enumeration = enumeration;
    if (m_enumeration && m_enumeration->size() > 0)
        m_value = m_enumeration->itemAt(0).value;
}

QByteArray PropertyEnumeration::name() const
{
    return m_enumeration ? m_enumeration->findName(m_value) : QByteArray();
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
    return QVariant::fromValue(this->name());
}

Result<void> PropertyEnumeration::setValueFromVariant(const QVariant& value)
{
    if (m_enumeration) {
        const QByteArray name = value.toByteArray();
        const Enumeration::Item* ptrItem = findEnumerationItem(*m_enumeration, name);
        if (ptrItem)
            return this->setValue(ptrItem->value);
    }

    return Result<void>::error();
}

const char* PropertyEnumeration::dynTypeName() const
{
    return PropertyEnumeration::TypeName;
}

const char PropertyEnumeration::TypeName[] = "Mayo::PropertyEnumeration";

} // namespace Mayo
