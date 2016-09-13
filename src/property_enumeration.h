#pragma once

#include "property.h"
#include <utility>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Enumeration
{
public:
    typedef int Value;
    struct Mapping {
        Value value;
        QString string;
    };

    Enumeration() = default;

    void map(int eval, const QString& str);

    std::size_t size() const;
    std::size_t index(int eval) const;

    Value valueAt(std::size_t i) const;
    Value value(const QString& str) const;
    const QString& string(Value eval) const;

    Mapping mapping(std::size_t i) const;
    const std::vector<Mapping>& mappings() const;

private:
    std::vector<Mapping>::const_iterator findCppSql(Value eval) const;

    std::vector<Mapping> m_vecMapping;
};

class PropertyEnumeration : public Property
{
public:
    PropertyEnumeration(
            PropertyOwner* owner,
            const QString& label,
            const Enumeration* enumeration);

    const Enumeration& enumeration() const;

    const QString& string() const;
    Enumeration::Value value() const;
    template<typename T> T valueAs() const;
    void setValue(Enumeration::Value v);

    const char* dynTypeName() const override;

private:
    Enumeration::Value m_value;
    const Enumeration* m_enumeration;
};

const Enumeration& enum_Graphic3dNameOfMaterial();



// -- Implementation

template<typename T> T PropertyEnumeration::valueAs() const
{ return static_cast<T>(m_value); }

} // namespace Mayo
