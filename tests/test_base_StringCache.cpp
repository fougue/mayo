/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/string_cache.h"

namespace Mayo {

void TestBase::StringCache_addFirstTimeThenCachedReturnsSameViewAndFlag_test()
{
    StringCache cache;

    std::string_view str{"hello"};
    bool already = true;
    auto cachedStr1 = cache.add(str, &already);
    QVERIFY(!already);
    QCOMPARE(cachedStr1.size(), str.size());
    QCOMPARE(cachedStr1, str);

    already = false;
    auto cachedStr2 = cache.add(str, &already);
    QVERIFY(already);

    QCOMPARE(cachedStr1.size(), cachedStr2.size());
    QCOMPARE(cachedStr1.data(), cachedStr2.data());
}

void TestBase::StringCache_addTwoSmallStringsContiguous_test()
{
    StringCache cache;
    cache.setPoolSize(64);

    std::string_view str1{"abc"};
    std::string_view str2{"defg"};
    QVERIFY(cache.poolSize() > (str1.size() + str2.size()));

    auto cachedStr1 = cache.add(str1);
    auto cachedStr2 = cache.add(str2);

    QVERIFY(!cachedStr1.empty() && !cachedStr2.empty());
    QCOMPARE(cachedStr1.size(), str1.size());
    QCOMPARE(cachedStr2.size(), str2.size());
    // Sequential concat in same pool
    QCOMPARE(cachedStr1.data() + cachedStr1.size(), cachedStr2.data());

    // Sanity check on contents
    QCOMPARE(cachedStr1, str1);
    QCOMPARE(cachedStr2, str2);
}

void TestBase::StringCache_addWhenRemainingCapacityInsufficientCreatesNewPool_test()
{
    // Set too small pool size to fit two consecutive strings
    StringCache cache;
    cache.setPoolSize(128);

    const std::string str1(size_t(120), 'a');
    const std::string str2(size_t(20), 'B');

    auto cachedStr1 = cache.add(str1); // Should fit in first pool
    auto cachedStr2 = cache.add(str2); // New pool expected

    QVERIFY(!cachedStr1.empty() && !cachedStr2.empty());
    // The two cached strings must not be in the same pool
    QVERIFY((cachedStr1.data() + cachedStr1.size()) != cachedStr2.data());
    // Not at same address
    QVERIFY(cachedStr1.data() != cachedStr2.data());

    QCOMPARE(cachedStr1, str1);
    QCOMPARE(cachedStr2, str2);
}

void TestBase::StringCache_addWithNullAlreadyCachedPtrIsOK_test()
{
    StringCache cache;

    std::string_view str{"xyz"};
    auto cachedStr1 = cache.add(str, nullptr);
    QCOMPARE(cachedStr1, str);

    auto cachedStr2 = cache.add(str, nullptr);
    QCOMPARE(cachedStr1.data(), cachedStr2.data());
}

void TestBase::StringCache_clearThenReAddReturnsNewViewAndNotCached_test()
{
    StringCache cache;

    std::string_view str{"abc"};
    {
        bool already = true;
        auto cachedStr = cache.add(str, &already);
        QVERIFY(!already);
        QCOMPARE(cachedStr, str);
    }

    cache.clear();

    {
        bool already = true;
        auto cachedStr = cache.add(str, &already);
        QVERIFY(!already); // After clear(), must no longer be cached
        QCOMPARE(cachedStr, str);
    }
}

void TestBase::StringCache_viewRemainsValidAfterSubsequentAddsWithinCapacity_test()
{
    StringCache cache;
    cache.setPoolSize(64);

    std::string_view str{"anchor"};
    auto v0 = cache.add(str);

    // Add several small strings fitting in current pool
    std::string_view smalls[] = {"a", "bb", "ccc", "dddd", "ee", "f", "g"};
    size_t total = 0;
    for (const std::string_view& s : smalls) {
        total += s.size();
        cache.add(s);
    }
    QVERIFY(total + v0.size() < cache.poolSize());

    // Initial cached string must still point to `str`
    QCOMPARE(v0, str);
}

void TestBase::StringCache_longStringLargerThanPoolWorksAndNextInsertsOK_test()
{
    StringCache cache;
    cache.setPoolSize(64); // Small poolSize

    // String bigger than pool
    const std::string longStr(size_t(64 * 5), 'A');

    bool already = true;
    auto vlong = cache.add(longStr, &already);

    QVERIFY(!already);
    QCOMPARE(vlong.size(), longStr.size());
    QCOMPARE(vlong, longStr);

    // Cache small string and check StringCache remains coherent
    auto v2 = cache.add("ok");
    QCOMPARE(v2, "ok");
}

void TestBase::StringCache_stressFuzzRandomStringsDedupAndClear_test()
{
    StringCache cache;
    cache.setPoolSize(2048);

    size_t N = 2000; // Count of random insertions
    std::vector<std::string> saved;
    saved.reserve(N);

    // Pseudo-radom generator
    std::mt19937 rng(1337/*arbitrary seed*/);
    std::uniform_int_distribution<int> lenDist(1, 40);
    std::uniform_int_distribution<int> charDist(0, 25);

    // First pass: insertion
    for (int i = 0; i < N; ++i) {
        const int len = lenDist(rng);
        std::string s;
        s.reserve(len);
        for (int j = 0; j < len; ++j)
            s.push_back('a' + charDist(rng));

        // Avoid collisions
        if (std::find(saved.cbegin(), saved.cend(), s) != saved.cend())
            break;

        saved.push_back(s);

        bool already = true;
        auto v = cache.add(s, &already);
        QVERIFY(!already);
        QCOMPARE(v, s);
    }

    N = saved.size();
    // Verifying caching
    for (int k = 0; k < 100; ++k) {
        int idx = rng() % N;
        const std::string& s = saved[idx];
        bool already = false;
        auto v = cache.add(s, &already);
        QVERIFY(already);
        QCOMPARE(v, s);
    }

    // Clear: invalidates cache
    cache.clear();

    // Re-insert after clear()
    std::vector<int> idxVisited;
    for (int k = 0; k < 100; ++k) {
        const int idx = rng() % N;
        if (std::find(idxVisited.cbegin(), idxVisited.cend(), idx) == idxVisited.cend())
            break;

        const std::string& s = saved[idx];
        idxVisited.push_back(idx);
        bool already = true;
        auto v = cache.add(s, &already);
        QVERIFY(!already);
        QCOMPARE(v, s);
    }
}

} // namespace Mayo
