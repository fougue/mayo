#include "property_enumeration.h"

#include "options.h"

#include <cassert>

namespace Mayo {

void Enumeration::map(Value eval, const QString& str)
{
    m_vecMapping.emplace_back(eval, str);
}

std::size_t Enumeration::size() const
{
    return m_vecMapping.size();
}

std::size_t Enumeration::index(Value eval) const
{
    return this->findCppSql(eval) - m_vecMapping.cbegin();
}

Enumeration::Value Enumeration::valueAt(std::size_t i) const
{
    return m_vecMapping.at(i).first;
}

Enumeration::Value Enumeration::value(const QString& str) const
{
    auto it = std::find_if(
                m_vecMapping.cbegin(),
                m_vecMapping.cend(),
                [&](const Mapping& mapping) { return mapping.second == str; });
    assert(it != m_vecMapping.cend());
    return it->first;
}

const QString& Enumeration::string(Value eval) const
{
    auto it = this->findCppSql(eval);
    assert(it != m_vecMapping.cend());
    return it->second;
}

Enumeration::Mapping Enumeration::mapping(std::size_t i) const
{
    assert(i < m_vecMapping.size());
    return m_vecMapping.at(i);
}

std::vector<Enumeration::Mapping>::const_iterator
Enumeration::findCppSql(Value eval) const
{
    auto it = std::find_if(
                m_vecMapping.cbegin(),
                m_vecMapping.cend(),
                [=] (const Mapping& map) { return map.first == eval; } );
    assert(it != m_vecMapping.cend());
    return it;
}

PropertyEnumeration::PropertyEnumeration(
        PropertyOwner* owner,
        const QString& label,
        const Enumeration* enumeration)
    : Property(owner, label),
      m_enumeration(enumeration)
{
    assert(m_enumeration != nullptr);
    assert(m_enumeration->size() > 0);
    m_value = m_enumeration->valueAt(0);
}

const Enumeration& PropertyEnumeration::enumeration() const
{
    return *m_enumeration;
}

const QString& PropertyEnumeration::string() const
{
    return m_enumeration->string(m_value);
}

Enumeration::Value PropertyEnumeration::value() const
{
    return m_value;
}

void PropertyEnumeration::setValue(Enumeration::Value v)
{
    // TODO: check v is an enumerated value of m_enumeration
    m_value = v;
    this->notifyChanged();
}

const char* PropertyEnumeration::dynTypeName() const
{
    return Property::EnumerationTypeName;
}

const Enumeration &enum_Graphic3dNameOfMaterial()
{
    static Enumeration enumeration;
    if (enumeration.size() == 0) {
        for (const Options::Material& mat : Options::materials())
            enumeration.map(mat.code, mat.name);
    }
    return enumeration;
}

} // namespace Mayo
