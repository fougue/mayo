/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "string_cache.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <list>
#include <vector>
#include <unordered_set>

namespace Mayo {

using StringPool = std::vector<char>;

struct StringCache::Pimpl {
    std::list<StringPool> stringPools;
    std::unordered_set<std::string_view> stringCache;
    unsigned poolSize = StringCache::defaultPoolSize;
};

StringCache::StringCache()
{
}

StringCache::~StringCache()
{
    delete d;
}

void StringCache::createPimpl()
{
    if (!d)
        d = new Pimpl;
}

unsigned StringCache::poolSize() const
{
    return d ? d->poolSize : StringCache::defaultPoolSize;
}

void StringCache::setPoolSize(unsigned s)
{
    this->createPimpl();
    d->poolSize = s > 0 ? s : StringCache::defaultPoolSize;
}

void StringCache::clear()
{
    delete d;
    d = nullptr;
}

std::string_view StringCache::add(std::string_view str, bool* alreadyCached)
{
    this->createPimpl();

    // Look cache
    {
        auto it = d->stringCache.find(str);
        if (it != d->stringCache.cend()) {
            if (alreadyCached)
                *alreadyCached = true;

            return *it;
        }
    }

    auto itPool = std::find_if(d->stringPools.begin(), d->stringPools.end(), [=](const StringPool& pool) {
        return (pool.capacity() - pool.size()) >= str.size();
    });
    // Allocate new pool if `str` doesn't fit in existing pools
    if (itPool == d->stringPools.end()) {
        StringPool pool;
        pool.reserve(std::max(size_t(d->poolSize), str.size()));
        d->stringPools.push_back(std::move(pool));
        itPool = std::prev(d->stringPools.end());
    }

    assert(itPool != d->stringPools.end());
    auto& pool = *itPool;

    // Append string to target pool
    const size_t poolSizeBeforeAdd = pool.size();
    assert(pool.capacity() >= (poolSizeBeforeAdd + str.size()));
    pool.resize(poolSizeBeforeAdd + str.size());
    std::copy(str.cbegin(), str.cend(), pool.begin() + poolSizeBeforeAdd);
    std::string_view vstr{pool.data() + poolSizeBeforeAdd, pool.size() - poolSizeBeforeAdd};
    // Cache string
    d->stringCache.insert(vstr);
    if (alreadyCached)
        *alreadyCached = false;

    return vstr;
}

} // namespace Mayo
