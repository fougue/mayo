/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "enumeration.h"
#include "enumeration_fromenum.h"
#include "property.h"

#include <string_view>
#include <type_traits>

// Silent GCC warnings about PropertyEnum<>::m_enum being initialized before PropertyEnumeration
// attributes
#ifdef Q_CC_GNU
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wreorder"
#endif

namespace Mayo {

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(PropertyGroup* grp, const TextId& name, const Enumeration& enumeration);

    constexpr const Enumeration& enumeration() const { return m_enumeration; }

    bool descriptionsEmpty() const { return m_vecDescription.empty(); }
    void addDescription(Enumeration::Value value, std::string_view descr);
    std::string_view findDescription(Enumeration::Value value) const;
    void clearDescriptions();

    std::string_view name() const;
    Enumeration::Value value() const;
    bool setValue(Enumeration::Value value);
    bool setValueByName(std::string_view name);

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    struct Description {
        Enumeration::Value value;
        std::string text;
    };

    const Enumeration& m_enumeration;
    Enumeration::Value m_value;
    std::vector<Description> m_vecDescription;
};

template<typename ENUM>
class PropertyEnum : public PropertyEnumeration {
    static_assert(std::is_enum<ENUM>::value, "ENUM must be an enumeration type");
public:
    using EnumType = ENUM;

    PropertyEnum(PropertyGroup* grp, const TextId& name);

    Enumeration& mutableEnumeration() { return m_enum; }

    EnumType value() const;
    operator EnumType() const { return this->value(); }
    bool setValue(EnumType value);

    void addDescription(EnumType value, std::string_view descr);
    void setDescriptions(std::initializer_list<std::pair<EnumType, std::string_view>> initList);

private:
    Enumeration m_enum;
};



// -- Implementation

template<typename ENUM>
PropertyEnum<ENUM>::PropertyEnum(PropertyGroup* grp, const TextId& name)
    : m_enum(Enumeration::fromType<EnumType>()),
      PropertyEnumeration(grp, name, m_enum)
{ }

template<typename ENUM>
ENUM PropertyEnum<ENUM>::value() const {
    return static_cast<EnumType>(PropertyEnumeration::value());
}

template<typename ENUM>
bool PropertyEnum<ENUM>::setValue(EnumType value) {
    return PropertyEnumeration::setValue(static_cast<Enumeration::Value>(value));
}

template<typename ENUM>
void PropertyEnum<ENUM>::addDescription(EnumType value, std::string_view descr) {
    PropertyEnumeration::addDescription(static_cast<Enumeration::Value>(value), descr);
}

template<typename ENUM>
void PropertyEnum<ENUM>::setDescriptions(std::initializer_list<std::pair<EnumType, std::string_view>> initList)
{
    this->clearDescriptions();
    for (const auto& [value, description] : initList)
        this->addDescription(value, description);
}

} // namespace Mayo

#ifdef Q_CC_GNU
#  pragma GCC diagnostic pop
#endif
