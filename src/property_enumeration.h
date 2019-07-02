/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "span.h"
#include <utility>
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Enumeration {
public:
    using Value = int;
    struct Mapping {
        Value value;
        QString string;
    };

    Enumeration() = default;

    void map(int eval, const QString& str);

    size_t size() const;
    size_t index(int eval) const;

    Value valueAt(size_t i) const;
    Value value(const QString& str) const;
    const QString& string(Value eval) const;

    Mapping mapping(size_t i) const;
    Span<const Mapping> mappings() const;

private:
    std::vector<Mapping>::const_iterator findCppSql(Value eval) const;
    std::vector<Mapping> m_vecMapping;
};

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(
            PropertyOwner* owner,
            const QString& label,
            const Enumeration* enumeration);

    const Enumeration& enumeration() const;

    const QString& string() const;
    Enumeration::Value value() const;
    template<typename T> T valueAs() const;
    Result<void> setValue(Enumeration::Value v);

    QVariant valueAsVariant() const override;
    Result<void> setValueFromVariant(const QVariant& value) override;

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    Enumeration::Value m_value;
    const Enumeration* m_enumeration;
};

const Enumeration& enum_Graphic3dNameOfMaterial();
const Enumeration& enum_AspectHatchStyle();



// -- Implementation

template<typename T> T PropertyEnumeration::valueAs() const
{ return static_cast<T>(m_value); }

} // namespace Mayo
