/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "property.h"

#include "property_enumeration.h"
#include <cassert>

namespace Mayo {

const std::vector<Property*>& PropertyOwner::properties() const
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

const QString &Property::label() const
{
    return m_label;
}

Property::Property(PropertyOwner *owner, const QString &label)
    : m_owner(owner),
      m_label(label)
{
    m_owner->addProperty(this);
}

void Property::notifyChanged()
{
    if (!m_owner->isPropertyChangedBlocked())
        m_owner->onPropertyChanged(this);
}

const char Property::BoolTypeName[] = "Mayo::PropertyBool";
const char Property::IntTypeName[] = "Mayo::PropertyInt";
const char Property::DoubleTypeName[] = "Mayo::PropertyDouble";
const char Property::QByteArrayTypeName[] = "Mayo::PropertyQByteArray";
const char Property::QStringTypeName[] = "Mayo::PropertyQString";
const char Property::QDateTimeTypeName[] = "Mayo::PropertyQDateTime";
const char Property::EnumerationTypeName[] = "Mayo::PropertyEnumeration";
const char Property::OccColorTypeName[] = "Mayo::PropertyOccColor";

PropertyOwner::PropertyChangedBlocker::PropertyChangedBlocker(PropertyOwner *owner)
    : m_owner(owner)
{
    m_owner->blockPropertyChanged(true);
}

PropertyOwner::PropertyChangedBlocker::~PropertyChangedBlocker()
{
    m_owner->blockPropertyChanged(false);
}

} // namespace Mayo
