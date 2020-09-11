/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "span.h"

#include <initializer_list>

namespace Mayo {

class Enumeration {
public:
    using Value = int;
    struct Item {
        Value value;
        QByteArray name;
        QString text;
    };

    Enumeration() = default;
    Enumeration(std::initializer_list<Item> listItem);

    void addItem(Value value, const QByteArray& name, const QString& text = QString());
    int size() const;

    const Item& findItem(Value value) const;
    int findIndex(Value value) const;
    QByteArray findName(Value value) const;
    Value findValue(const QByteArray& name) const;

    const Item& itemAt(int index) const;
    Span<const Item> items() const;

private:
    std::vector<Item> m_vecItem;
};

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(
            PropertyOwner* owner,
            const QString& label,
            const Enumeration* enumeration = nullptr);

    const Enumeration* enumeration() const;
    void setEnumeration(const Enumeration* enumeration);

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



// -- Implementation

template<typename T> T PropertyEnumeration::valueAs() const
{ return static_cast<T>(m_value); }

} // namespace Mayo
