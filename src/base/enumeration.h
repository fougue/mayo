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
#include <type_traits>

namespace Mayo {

// Provides meta-data about enumerated values
class Enumeration {
public:
    // Common type to store enumerated values
    using Value = int;

    // Enumeration item being a value-name pair
    struct Item {
        Value value;
        TextId name;
    };

    // Ctors
    Enumeration() = default;
    Enumeration(std::initializer_list<Item> listItem);

    // Adds an enumerated item
    template<typename ValueType>
    Enumeration& addItem(ValueType value, const TextId& name);

    // Iterates over name of items and removes 'prefix' string that may appear at the beginning
    Enumeration& chopPrefix(std::string_view prefix);

    // Assigns 'context' to TextId::trContext of all Item objects
    Enumeration& changeTrContext(std::string_view context);

    // Count of enumeration items
    int size() const { return int(m_vecItem.size()); }
    bool empty() const { return m_vecItem.empty(); }

    // Finds index of the item corresponding to an enumerated value. Returns -1 if not found
    template<typename EnumType> int findIndexByValue(EnumType value) const;

    // Finds the item corresponding to an enumerated value. Returns nullptr if not found
    template<typename EnumType> const Item* findItemByValue(EnumType value) const;

    // Finds the item corresponding to a name. Returns nullptr if not found
    const Item* findItemByName(std::string_view name) const;

    // Finds the name of an enumerated value. Returns empty string if not found
    template<typename EnumType> std::string_view findNameByValue(EnumType value) const;

    // Finds the enumerated value corresponding to a name. Throws exception if not found
    Value findValueByName(std::string_view name) const;

    // Returns 'true' if there is an item matching 'name'
    bool contains(std::string_view name) const;

    // Returns item at 'index'
    const Item& itemAt(int index) const { return m_vecItem.at(index); }

    // Returns read-only array of the items
    Span<const Item> items() const { return m_vecItem; }

    // Creates an Enumeration object from an enumerated type, using MetaEnum helper
    // Note: client code has to include header "enumeration_fromenum.h"
    template<typename EnumType>
    static Enumeration fromType();

private:
    int findIndexByValue_untyped(Value value) const;

    std::vector<Item> m_vecItem;
};



// -- Implementation

template<typename ValueType> Enumeration& Enumeration::addItem(ValueType value, const TextId& name)
{
    const Item item = { Enumeration::Value(value), name };
    m_vecItem.emplace_back(std::move(item));
    return *this;
}

template<typename EnumType> int Enumeration::findIndexByValue(EnumType value) const
{
    static_assert(std::is_enum_v<EnumType> || std::is_integral_v<EnumType>, "ENUM must be an enumeration or integer type");
    return this->findIndexByValue_untyped(static_cast<Enumeration::Value>(value));
}

template<typename EnumType> const Enumeration::Item* Enumeration::findItemByValue(EnumType value) const
{
    static_assert(std::is_enum_v<EnumType> || std::is_integral_v<EnumType>, "EnumType must be an enumeration or integer type");
    const int index = this->findIndexByValue_untyped(static_cast<Enumeration::Value>(value));
    return index != -1 ? &(this->itemAt(index)) : nullptr;
}

template<typename EnumType> std::string_view Enumeration::findNameByValue(EnumType value) const
{
    const int index = this->findIndexByValue(value);
    if (index != -1)
        return this->itemAt(index).name.key;
    else
        return {};
}

} // namespace Mayo
