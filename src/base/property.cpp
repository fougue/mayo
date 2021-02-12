/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property.h"

#include "property_enumeration.h"
#include <cassert>

namespace Mayo {

PropertyGroup::PropertyGroup(PropertyGroup* parentGroup)
    : m_parentGroup(parentGroup)
{
}

Span<Property* const> PropertyGroup::properties() const
{
    return m_properties;
}

void PropertyGroup::restoreDefaults()
{
}

void PropertyGroup::onPropertyChanged(Property* prop)
{
    if (m_parentGroup)
        m_parentGroup->onPropertyChanged(prop);
}

Result<void> PropertyGroup::isPropertyValid(const Property*) const
{
    return Result<void>::ok();
}

void PropertyGroup::blockPropertyChanged(bool on)
{
    m_propertyChangedBlocked = on;
}

bool PropertyGroup::isPropertyChangedBlocked() const
{
    return m_propertyChangedBlocked;
}

void PropertyGroup::addProperty(Property* prop)
{
    Expects(prop != nullptr);
    Expects(prop->group() == this);
    m_properties.emplace_back(prop);
}

void PropertyGroup::removeProperty(Property* prop)
{
    auto it = std::find(m_properties.begin(), m_properties.end(), prop);
    if (it != m_properties.end())
        m_properties.erase(it);
}

const TextId& Property::name() const
{
    return m_name;
}

QString Property::label() const
{
    return m_name.tr();
}

Property::Property(PropertyGroup* group, const TextId& name)
    : m_group(group),
      m_name(name)
{
    if (m_group)
        m_group->addProperty(this);
}

void Property::notifyChanged()
{
    if (m_group && !m_group->isPropertyChangedBlocked())
        m_group->onPropertyChanged(this);
}

Result<void> Property::isValid() const
{
    if (m_group)
        return m_group->isPropertyValid(this);

    return Result<void>::ok();
}

bool Property::hasGroup() const
{
    return m_group != nullptr;
}


PropertyChangedBlocker::PropertyChangedBlocker(PropertyGroup* group)
    : m_group(group)
{
    if (m_group)
        m_group->blockPropertyChanged(true);
}

PropertyChangedBlocker::~PropertyChangedBlocker()
{
    if (m_group)
        m_group->blockPropertyChanged(false);
}


PropertyGroupSignals::PropertyGroupSignals(QObject* parent)
    : QObject(parent)
{
}

void PropertyGroupSignals::onPropertyChanged(Property* prop)
{
    PropertyGroup::onPropertyChanged(prop);
    emit propertyChanged(prop);
}

} // namespace Mayo
