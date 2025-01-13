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
    // This constructor should be used when 'enumeration' is guaranteed to be fully constructed
    PropertyEnumeration(PropertyGroup* grp, const TextId& name, const Enumeration* enumeration);

    const Enumeration& enumeration() const;

    bool descriptionsEmpty() const { return m_vecDescription.empty(); }
    void addDescription(Enumeration::Value value, std::string_view descr);
    std::string_view findDescription(Enumeration::Value value) const;
    void clearDescriptions();

    std::string_view valueName() const;
    Enumeration::Value value() const;
    bool setValue(Enumeration::Value value);
    bool setValueByName(std::string_view name);

    const char* dynTypeName() const override;
    static const char TypeName[];

protected:
    // This constructor should be used when 'enumeration' isn't completely constructed, as in
    // PropertyEnum<> subclass. The enumeration member is set with the setEnumeration() helper
    PropertyEnumeration(PropertyGroup* grp, const TextId& name);

    void setEnumeration(const Enumeration* enumeration);

private:
    struct Description {
        Enumeration::Value value;
        std::string text;
    };

    const Enumeration* m_enumeration = nullptr;
    Enumeration::Value m_value = -1;
    std::vector<Description> m_vecDescription;
};

template<typename EnumType>
class PropertyEnum : public PropertyEnumeration {
    static_assert(std::is_enum<EnumType>::value, "ENUM must be an enumeration type");
public:
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

template<typename EnumType>
PropertyEnum<EnumType>::PropertyEnum(PropertyGroup* grp, const TextId& name)
    : PropertyEnumeration(grp, name),
      m_enum(Enumeration::fromType<EnumType>())
{
    this->setEnumeration(&m_enum);
}

template<typename EnumType>
EnumType PropertyEnum<EnumType>::value() const {
    return static_cast<EnumType>(PropertyEnumeration::value());
}

template<typename EnumType>
bool PropertyEnum<EnumType>::setValue(EnumType value) {
    return PropertyEnumeration::setValue(static_cast<Enumeration::Value>(value));
}

template<typename EnumType>
void PropertyEnum<EnumType>::addDescription(EnumType value, std::string_view descr) {
    PropertyEnumeration::addDescription(static_cast<Enumeration::Value>(value), descr);
}

template<typename EnumType>
void PropertyEnum<EnumType>::setDescriptions(std::initializer_list<std::pair<EnumType, std::string_view>> initList)
{
    this->clearDescriptions();
    for (const auto& [val, descr] : initList)
        this->addDescription(val, descr);
}

} // namespace Mayo

#ifdef Q_CC_GNU
#  pragma GCC diagnostic pop
#endif
