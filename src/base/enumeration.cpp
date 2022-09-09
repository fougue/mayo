/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "enumeration.h"

#include "cpp_utils.h"

#include <fmt/format.h>
#include <algorithm>
#include <cassert>
#include <exception>

namespace Mayo {

Enumeration::Enumeration(std::initializer_list<Item> listItem)
    : m_vecItem(listItem)
{
}

Enumeration& Enumeration::chopPrefix(std::string_view prefix)
{
    for (Item& item : m_vecItem) {
        if (item.name.key.substr(0, prefix.size()) == prefix) // TODO Replace with C++20 starts_with()
            item.name.key = item.name.key.substr(prefix.size());
    }

    return *this;
}

Enumeration& Enumeration::changeTrContext(std::string_view context)
{
    for (Item& item : m_vecItem)
        item.name.trContext = context;

    return *this;
}

int Enumeration::findIndexByValue_untyped(Value value) const
{
    auto it = std::find_if(m_vecItem.cbegin(), m_vecItem.cend(), [=](const Item& item) {
        return item.value == value;
    });
    return it != m_vecItem.cend() ? CppUtils::safeStaticCast<int>(it - m_vecItem.cbegin()) : -1;
}

Enumeration::Value Enumeration::findValueByName(std::string_view name) const
{
    const Enumeration::Item* ptrItem = this->findItemByName(name);
    if (!ptrItem)
        throw std::runtime_error(fmt::format("No matching enumeration item found [name={}]", name));

    return ptrItem->value;
}

bool Enumeration::contains(std::string_view name) const
{
    return this->findItemByName(name) != nullptr;
}

const Enumeration::Item* Enumeration::findItemByName(std::string_view name) const
{
    auto itFound = std::find_if(
                m_vecItem.cbegin(),
                m_vecItem.cend(),
                [&](const Enumeration::Item& enumItem) { return name == enumItem.name.key; });
    return itFound != m_vecItem.cend() ? &(*itFound) : nullptr;
}

} // namespace Mayo
