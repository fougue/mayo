/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <unordered_map>

namespace Mayo {

class CppUtils {
public:
    template<typename KEY, typename VALUE, typename... EXTRAS>
    static VALUE findValue(const KEY& key, const std::unordered_map<KEY, VALUE, EXTRAS...>& hashmap) {
        auto it = hashmap.find(key);
        return it != hashmap.cend() ? it->second : VALUE();
    }
};

} // namespace Mayo
