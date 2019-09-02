/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "span.h"
#include <Aspect_HatchStyle.hxx>
#include <Graphic3d_NameOfMaterial.hxx>

namespace Mayo {

class Enumeration {
public:
    using Value = int;
    struct Item {
        Value value;
        QString name;
    };

    Enumeration() = default;

    void addItem(Value value, const QString& name);
    int size() const;

    int findIndex(Value value) const;
    QString findName(Value value) const;
    Value findValue(const QString& name) const;

    Item itemAt(int index) const;
    Span<const Item> items() const;

private:
    std::vector<Item> m_vecItem;
};

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(
            PropertyOwner* owner,
            const QString& label,
            const Enumeration* enumeration);

    const Enumeration& enumeration() const;

    QString name() const;
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
