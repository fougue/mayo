/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "string_cache.h"
#include "text_id.h"

#include <shared_mutex>
#include <unordered_map>

namespace Mayo {

// Thread-safe cache for translated texts
//
// A translation is uniquely identified by its TextId (translation context + source text) and the
// plural form parameter `n`
// Cached strings are owned by the internal StringCache, so the returned std::string_view remains
// valid until this cache is cleared or destroyed
// Concurrent lookups are allowed. Insertions and clearing are mutually exclusive with lookups and
// other modifications
class I18nTranslationCache {
public:
    // Returns the cached translation for `text` and plural form `n`, or an empty view if no
    // translation is cached
    std::string_view find(const TextId& text, int n) const;

    // Adds a translation to the cache and returns the cached translation
    // If a translation for `text` and `n` is already cached, the existing translation is returned
    // and `translation` is ignored
    std::string_view insert(const TextId& text, int n, std::string_view translation);

    // Removes all cached translations
    // std::string_views previously returned by find() or insert() must not be used after this
    // function has been called
    void clear();

private:
    // Identifies a cached translation
    struct Key {
        std::string_view trContext;
        std::string_view srcText;
        int n = 0;
        friend bool operator==(const Key& lhs, const Key& rhs) noexcept {
            return lhs.trContext == rhs.trContext && lhs.srcText == rhs.srcText && lhs.n == rhs.n;
        }
    };

    struct KeyHash {
        std::size_t operator()(const Key& key) const noexcept;
    };

    // Finds a cached translation without acquiring `m_mutex`
    // The caller must hold `m_mutex` with an appropriate lock
    std::string_view unlockedFind(const Key& key) const;

    mutable std::shared_mutex m_mutex;
    StringCache m_strCache;
    std::unordered_map<Key, std::string_view, KeyHash> m_mapTr;
};

} // namespace Mayo
