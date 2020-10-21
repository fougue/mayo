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
        TextId name;
        QString description;
    };

    Enumeration() = default;
    Enumeration(std::initializer_list<Item> listItem);

    template<typename VALUE>
    void addItem(VALUE value, const TextId& name, const QString& description = QString());

    int size() const;
    bool empty() const { return this->size() == 0; }

    bool descriptionsEmpty() const;
    template<typename VALUE> void setDescription(VALUE value, const QString& descr);

    const Item& findItem(Value value) const;
    int findIndex(Value value) const;
    QByteArray findName(Value value) const;
    Value findValue(const QByteArray& name) const;
    bool contains(const QByteArray& name) const;

    const Item& itemAt(int index) const;
    Span<const Item> items() const;

    template<typename QENUM>
    static Enumeration fromQENUM(const QByteArray& textIdContext);

private:
    std::vector<Item> m_vecItem;
};

class PropertyEnumeration : public Property {
public:
    PropertyEnumeration(
            PropertyGroup* grp,
            const TextId& name,
            const Enumeration* enumeration = nullptr);

    const Enumeration* enumeration() const;
    void setEnumeration(const Enumeration* enumeration);

    QByteArray name() const;
    Enumeration::Value value() const;
    template<typename T> T valueAs() const;
    template<typename T> Result<void> setValue(T value);

    QVariant valueAsVariant() const override;
    Result<void> setValueFromVariant(const QVariant& value) override;

    const char* dynTypeName() const override;
    static const char TypeName[];

private:
    Enumeration::Value m_value;
    const Enumeration* m_enumeration;
};

} // namespace Mayo




// -- Implementation

#include <QtCore/QMetaEnum>

namespace Mayo {

template<typename VALUE> void Enumeration::addItem(
        VALUE value, const TextId& name, const QString& description)
{
    const Item item = { Enumeration::Value(value), name, description };
    m_vecItem.emplace_back(std::move(item));
}

template<typename VALUE> void Enumeration::setDescription(VALUE value, const QString& descr)
{
    const int index = this->findIndex(Enumeration::Value(value));
    if (index != -1)
        m_vecItem.at(index).description = descr;
}

template<typename QENUM>
Enumeration Enumeration::fromQENUM(const QByteArray& textIdContext)
{
    auto fnQByteArrayFrowRawData = [](const char* str) {
        return QByteArray::fromRawData(str, int(std::strlen(str)));
    };

    Enumeration enumObject;
    const QMetaEnum metaEnum = QMetaEnum::fromType<QENUM>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const char* strKey = metaEnum.key(i);
        const TextId keyTextId = { textIdContext, fnQByteArrayFrowRawData(strKey) };
        enumObject.addItem(metaEnum.value(i), keyTextId);
    }

    return enumObject;
}

template<typename T> T PropertyEnumeration::valueAs() const {
    return static_cast<T>(m_value);
}

template<typename T> Result<void> PropertyEnumeration::setValue(T value) {
    // TODO: check v is an enumerated value of m_enumeration
    return Property::setValueHelper(this, &m_value, int(value));
}

} // namespace Mayo
