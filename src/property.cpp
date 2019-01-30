/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property.h"

#include "property_enumeration.h"
#include <cassert>

namespace Mayo {

Span<Property* const> PropertyOwner::properties() const
{
    return m_properties;
}

void PropertyOwner::onPropertyChanged(Property* /*prop*/)
{ }

void PropertyOwner::blockPropertyChanged(bool on)
{
    m_propertyChangedBlocked = on;
}

bool PropertyOwner::isPropertyChangedBlocked() const
{
    return m_propertyChangedBlocked;
}

void PropertyOwner::addProperty(Property* prop)
{
    m_properties.emplace_back(prop);
}

void PropertyOwner::removeProperty(Property* prop)
{
    auto it = std::find(m_properties.begin(), m_properties.end(), prop);
    if (it != m_properties.end())
        m_properties.erase(it);
}

const QString &Property::label() const
{
    return m_label;
}

bool Property::isUserReadOnly() const
{
    return m_isUserReadOnly;
}

void Property::setUserReadOnly(bool on)
{
    m_isUserReadOnly = on;
}

Property::Property(PropertyOwner* owner, const QString& label)
    : m_owner(owner),
      m_label(label)
{
    if (m_owner != nullptr)
        m_owner->addProperty(this);
}

void Property::notifyChanged()
{
    if (m_owner != nullptr && !m_owner->isPropertyChangedBlocked())
        m_owner->onPropertyChanged(this);
}



PropertyChangedBlocker::PropertyChangedBlocker(PropertyOwner *owner)
    : m_owner(owner)
{
    if (m_owner != nullptr)
        m_owner->blockPropertyChanged(true);
}

PropertyChangedBlocker::~PropertyChangedBlocker()
{
    if (m_owner != nullptr)
        m_owner->blockPropertyChanged(false);
}

HandleProperty::HandleProperty(HandleProperty &&other)
{
    this->swap(std::move(other));
}

HandleProperty &HandleProperty::operator=(HandleProperty &&other)
{
    this->swap(std::move(other));
    return *this;
}

HandleProperty::HandleProperty(Property *prop, HandleProperty::Storage storage)
    : m_prop(prop),
      m_storage(storage)
{
}

HandleProperty::~HandleProperty()
{
    if (m_storage == HandleProperty::Owner)
        delete m_prop;
}

Property *HandleProperty::get() const
{
    return m_prop;
}

Property *HandleProperty::operator->() const
{
    return m_prop;
}

HandleProperty::Storage HandleProperty::storage() const
{
    return m_storage;
}

void HandleProperty::swap(HandleProperty &&other)
{
    m_prop = other.m_prop;
    m_storage = other.m_storage;
    other.m_prop = nullptr;
}

PropertyOwnerSignals::PropertyOwnerSignals(QObject* parent)
    : QObject(parent)
{
}

void PropertyOwnerSignals::onPropertyChanged(Property* prop)
{
    PropertyOwner::onPropertyChanged(prop);
    emit propertyChanged(prop);
}

} // namespace Mayo
