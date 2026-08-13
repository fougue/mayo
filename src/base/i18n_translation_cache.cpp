/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "i18n_translation_cache.h"

namespace Mayo {

std::string_view I18nTranslationCache::find(const TextId& text, int n) const
{
    [[maybe_unused]] std::shared_lock lock(m_mutex);
    return this->unlockedFind(Key{text.trContext, text.key, n});
}

std::string_view I18nTranslationCache::insert(const TextId& text, int n, std::string_view translation)
{
    [[maybe_unused]] std::unique_lock lock(m_mutex);
    const Key lookupKey{text.trContext, text.key, n};
    std::string_view tr = this->unlockedFind(lookupKey);
    if (!tr.empty())
        return tr;

    const auto cachedTr = m_strCache.add(translation);
    const auto [it, ok] = m_mapTr.emplace(lookupKey, cachedTr);
    return it->second;
}

void I18nTranslationCache::clear()
{
    [[maybe_unused]] std::unique_lock lock(m_mutex);
    m_mapTr.clear();
    m_strCache.clear();
}

std::string_view I18nTranslationCache::unlockedFind(const Key& key) const
{
    const auto it = m_mapTr.find(key);
    if (it != m_mapTr.cend())
        return it->second;

    return {};
}

std::size_t I18nTranslationCache::KeyHash::operator()(const Key& key) const noexcept
{
    std::size_t h = std::hash<std::string_view>{}(key.trContext);

    h ^= std::hash<std::string_view>{}(key.srcText)
         + std::size_t{0x9e3779b9}
         + (h << 6)
         + (h >> 2);

    h ^= std::hash<int>{}(key.n)
         + std::size_t{0x9e3779b9}
         + (h << 6)
         + (h >> 2);

    return h;
}

} // namespace Mayo
