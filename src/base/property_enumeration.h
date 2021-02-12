/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "enumeration.h"
#include "enumeration_fromenum.h"
#include "property.h"

#include <type_traits>

namespace Mayo {

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(PropertyGroup* grp, const TextId& name, const Enumeration& enumeration);

    constexpr const Enumeration& enumeration() const { return m_enumeration; }

    bool descriptionsEmpty() const { return m_vecDescription.empty(); }
    void addDescription(Enumeration::Value value, const QString& descr);
    QString findDescription(Enumeration::Value value) const;
    void clearDescriptions();

    QByteArray name() const;
    Enumeration::Value value() const;
    Result<void> setValue(Enumeration::Value value);
    Result<void> setValueByName(const QByteArray& name);

    QVariant valueAsVariant() const override;
    Result<void> setValueFromVariant(const QVariant& value) override;

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    struct Description {
        Enumeration::Value value;
        QString text;
    };

    Enumeration::Value m_value;
    const Enumeration& m_enumeration;
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
    Result<void> setValue(EnumType value);

    void addDescription(EnumType value, const QString& descr);
    void setDescriptions(std::initializer_list<std::pair<EnumType, QString>> initList);

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
Result<void> PropertyEnum<ENUM>::setValue(EnumType value) {
    return PropertyEnumeration::setValue(static_cast<Enumeration::Value>(value));
}

template<typename ENUM>
void PropertyEnum<ENUM>::addDescription(EnumType value, const QString& descr) {
    PropertyEnumeration::addDescription(static_cast<Enumeration::Value>(value), descr);
}

template<typename ENUM>
void PropertyEnum<ENUM>::setDescriptions(std::initializer_list<std::pair<EnumType, QString>> initList)
{
    this->clearDescriptions();
    for (const auto& pair : initList)
        this->addDescription(pair.first, pair.second);
}

} // namespace Mayo
