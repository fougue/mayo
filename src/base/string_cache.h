/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <memory>
#include <string_view>

namespace Mayo {

// Provides utility designed to efficiently deduplicate, store, and reuse immutable strings
// Returns stable std::string_view values whose lifetime matches that of the StringCache object itself
//
// Characteristics:
//   *) Zero-overhead deduplication: identical strings are stored once
//   *) Stable references: returned std::string_view values remain valid until clear() or object destruction
//   *) Efficient memory layout: pools grow append-only, minimizing fragmentation and allocation overhead
//   *) Lazy initialization: no memory is allocated until the first string is added or the pool size
//      is explicitly set
//
// Limitations
//   *) Not thread-safe: external synchronization is required if used from multiple threads
//   *) Views are invalidated on clear()
//   *) A single extremely long string(> poolSize) will cause creation of a pool sized exactly for
//      that string
//   *) Pools grow append-only; individual strings cannot be removed
class StringCache {
public:
    // Constructs an empty cache, no memory is allocated at this stage
    StringCache();

    // Destructs the cache and frees all internal memory pools.
    // All previously returned std::string_view values become invalid
    ~StringCache();

    // Non copyable
    StringCache(const StringCache&) = delete;
    StringCache& operator=(const StringCache&) = delete;

    // Movable
    StringCache(StringCache&& other) noexcept = default;
    StringCache& operator=(StringCache&& other) noexcept = default;

    // Returns the current size used when allocating new string pools
    unsigned poolSize() const;

    // Configures the minimal capacity of newly created storage pools
    // If s > 0, this value becomes the new pool size.
    // If s == 0, the pool size resets to the default(defaultPoolSize).
    // This does not modify existing pools; it only affects future allocations
    void setPoolSize(unsigned s);

    // Clears all internal data: all string pools are destroyed and all cached string views are removed
    // After calling clear(), all previously returned std::string_view values become invalid
    void clear();

    // Adds a string to the cache or retrieves the existing cached version
    // Parameter `alreadyCached` is an optional pointer set to true/false if the string was already
    // present or newly inserted
    // Complexity:
    //   *) Average O(1) on lookup
    //   *) Proportional to string length when adding
    //   *) Very efficient for repeated strings
    std::string_view add(std::string_view str, bool* alreadyCached = nullptr);

    static constexpr unsigned defaultPoolSize = 64 * 1024; // 64KB

private:
    void createPimpl();
    struct Pimpl;
    std::unique_ptr<Pimpl> d;
};

} // namespace Mayo
