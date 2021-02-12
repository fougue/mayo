/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include "text_id.h"
#include <initializer_list>
#include <vector>

namespace Mayo {

class Enumeration {
public:
    using Value = int;
    struct Item {
        Value value;
        TextId name;
    };

    Enumeration() = default;
    Enumeration(std::initializer_list<Item> listItem);

    template<typename VALUE>
    Enumeration& addItem(VALUE value, const TextId& name);

    Enumeration& chopPrefix(std::string_view strPrefix);
    Enumeration& changeTrContext(const QByteArray& context);

    int size() const { return int(m_vecItem.size()); }
    bool empty() const { return this->size() == 0; }

    const Item& findItem(Value value) const;
    int findIndex(Value value) const;
    QByteArray findName(Value value) const;
    Value findValue(const QByteArray& name) const;
    bool contains(const QByteArray& name) const;

    const Item& itemAt(int index) const { return m_vecItem.at(index); }
    Span<const Item> items() const { return m_vecItem; }
    const Item* findItem(const QByteArray& name) const;

    template<typename ENUM>
    static Enumeration fromType();

    static const Enumeration& null();

private:
    std::vector<Item> m_vecItem;
};



// -- Implementation

template<typename VALUE> Enumeration& Enumeration::addItem(VALUE value, const TextId& name)
{
    const Item item = { Enumeration::Value(value), name };
    m_vecItem.emplace_back(std::move(item));
    return *this;
}

} // namespace Mayo
