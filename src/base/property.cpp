/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "property.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

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

void PropertyGroup::onPropertyAboutToChange(Property* prop)
{
    this->signalPropertyAboutToChange.send(prop);
    if (m_parentGroup)
        m_parentGroup->onPropertyAboutToChange(prop);
}

void PropertyGroup::onPropertyChanged(Property* prop)
{
    this->signalPropertyChanged.send(prop);
    if (m_parentGroup)
        m_parentGroup->onPropertyChanged(prop);
}

void PropertyGroup::onPropertyEnabled(Property* prop, bool on)
{
    this->signalPropertyEnabled.send(prop, on);
    if (m_parentGroup)
        m_parentGroup->onPropertyEnabled(prop, on);
}

bool PropertyGroup::isPropertyValid(const Property*) const
{
    return true;
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

std::string_view Property::label() const
{
    return m_name.tr();
}

void Property::setEnabled(bool on)
{
    if (m_isEnabled != on) {
        m_isEnabled = on;
        this->notifyEnabled(on);
    }
}

Property::Property(PropertyGroup* group, const TextId& name)
    : m_group(group),
      m_name(name)
{
    if (m_group)
        m_group->addProperty(this);
}

uint64_t Property::userData() const
{
    if (!m_hasUserData)
        throw std::runtime_error("Property::userData() isn't available");

    return m_userData;
}

void Property::setUserData(uint64_t d)
{
    m_hasUserData = true;
    m_userData = d;
}

void Property::clearUserData()
{
    m_hasUserData = false;
}

void Property::notifyAboutToChange()
{
    if (m_group && !m_group->isPropertyChangedBlocked())
        m_group->onPropertyAboutToChange(this);
}

void Property::notifyChanged()
{
    if (m_group && !m_group->isPropertyChangedBlocked())
        m_group->onPropertyChanged(this);
}

void Property::notifyEnabled(bool on)
{
    if (m_group/* && !m_group->isPropertyChangedBlocked()*/)
        m_group->onPropertyEnabled(this, on);
}

bool Property::isValid() const
{
    if (m_group)
        return m_group->isPropertyValid(this);

    return true;
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

} // namespace Mayo
