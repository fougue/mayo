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
