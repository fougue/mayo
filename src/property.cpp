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

#if 0
namespace Internal {

static void convertTypeIfNeeded(QVariant* value, int valueType)
{
    if (value->type() != valueType || value->userType() != valueType)
        value->convert(valueType);
}

static bool isScalarValueType(int valueType)
{
    return valueType == QVariant::Int
            || valueType == QVariant::UInt
            || valueType == QVariant::LongLong
            || valueType == QVariant::ULongLong
            || valueType == QVariant::Double;
}

} // namespace Internal

Property::Property(PropertyOwner* owner, const QString& label, int valueType)
    : m_owner(owner),
      m_label(label),
      m_value(valueType, nullptr),
      m_valueType(valueType),
      m_isScalarValueType(Internal::isScalarValueType(valueType))
{
    assert(m_owner != nullptr);
    m_owner->addProperty(this);
}

Property::Property(
        PropertyOwner *owner,
        const QString &label,
        int valueType,
        const std::pair<QVariant, QVariant> &scalarRange)
    : Property(owner, label, valueType)
{
    assert(m_isScalarValueType);
    assert(scalarRange.first.isValid() && !scalarRange.first.isNull());
    assert(scalarRange.second.isValid() && !scalarRange.second.isNull());
    assert(scalarRange.first.canConvert(valueType));
    assert(scalarRange.second.canConvert(valueType));
    m_scalarMinValue = scalarRange.first;
    m_scalarMaxValue = scalarRange.second;
    Internal::convertTypeIfNeeded(&m_scalarMinValue, m_valueType);
    Internal::convertTypeIfNeeded(&m_scalarMaxValue, m_valueType);
}

Property::Property(
        PropertyOwner *owner,
        const QString &label,
        int valueType,
        const std::vector<Property::EnumeratedValue> &vecEnum)
    : Property(owner, label, valueType)
{
    assert(!vecEnum.empty());
    m_enumeration.reserve(vecEnum.size());
    for (EnumeratedValue enumVal : vecEnum) {
        assert(enumVal.value.canConvert(valueType));
        Internal::convertTypeIfNeeded(&enumVal.value, valueType);
        m_enumeration.emplace_back(std::move(enumVal));
    }
    m_value = vecEnum.front().value;
}

const QString &Property::label() const
{
    return m_label;
}

int Property::valueType() const
{
    return m_valueType;
}

bool Property::isScalarValueType() const
{
    return m_isScalarValueType;
}

bool Property::hasScalarRange() const
{
    return m_isScalarValueType
            && m_scalarMinValue.isValid()
            && !m_scalarMinValue.isNull();
}

const QVariant &Property::scalarMinValue() const
{
    return m_scalarMinValue;
}

const QVariant &Property::scalarMaxValue() const
{
    return m_scalarMaxValue;
}

bool Property::hasEnumeration() const
{
    return !m_enumeration.empty();
}

const std::vector<Property::EnumeratedValue> &Property::enumeration() const
{
    return m_enumeration;
}

const Property::EnumeratedValue &Property::enumValue() const
{
    auto itFound = std::find_if(
                m_enumeration.cbegin(),
                m_enumeration.cend(),
                [&](const EnumeratedValue& eval) { return eval.value == m_value; });
    assert(itFound != m_enumeration.cend());
    return *itFound;
}

const QVariant &Property::value() const
{
    return m_value;
}

void Property::setValue(const QVariant &v)
{
    assert(v.canConvert(m_valueType));
    if (this->hasEnumeration()) {
        QVariant tmp = v;
        Internal::convertTypeIfNeeded(&tmp, m_valueType);
        auto itFound = std::find_if(
                    m_enumeration.cbegin(),
                    m_enumeration.cend(),
                    [&](const EnumeratedValue& eval) { return eval.value == tmp; });
        assert(itFound != m_enumeration.cend());
        m_value = std::move(tmp);
    }
    else {
        m_value = v;
        Internal::convertTypeIfNeeded(&m_value, m_valueType);
    }
    this->notifyChanged();
}

void Property::notifyChanged()
{
    m_owner->onPropertyChanged(this);
}

Property::EnumeratedValue::EnumeratedValue(
        const QString &lbl, const QVariant &val)
    : label(lbl),
      value(val)
{
}
#endif

} // namespace Mayo
