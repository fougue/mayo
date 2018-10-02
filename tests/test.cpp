#include "test.h"

#include "../src/libtree.h"
#include "../src/result.h"
#include "../src/unit.h"
#include "../src/unit_system.h"

#include <QtCore/QtDebug>
#include <cmath>
#include <cstring>
#include <utility>
#include <iostream>
#include <sstream>

namespace Mayo {

// For the sake of QCOMPARE()
static bool operator==(
        const UnitSystem::TranslateResult& lhs,
        const UnitSystem::TranslateResult& rhs)
{
    return std::abs(lhs.value - rhs.value) < 1e-6
            && std::strcmp(lhs.strUnit, rhs.strUnit) == 0
            && std::abs(lhs.factor - rhs.factor) < 1e-6;
}

void Test::CafUtils_test()
{
    // TODO Add CafUtils::labelTag() test for multi-threaded safety
}

void Test::Quantity_test()
{
    const QuantityArea area = (10 * Quantity_Millimeter) * (5 * Quantity_Centimeter);
    QCOMPARE(area.value(), 500.);
    QCOMPARE((Quantity_Millimeter / 5.).value(), 1/5.);
}

namespace Result_test {

struct Data {
    static std::ostream* data_ostr;

    Data() {
        *data_ostr << 0;
    }
    Data(const Data& other) : foo(other.foo) {
        *data_ostr << 1;
    }
    Data(Data&& other) {
        foo = std::move(other.foo);
        *data_ostr << 2;
    }
    Data& operator=(const Data& other) {
        this->foo = other.foo;
        *data_ostr << 3;
        return *this;
    }
    Data& operator=(Data&& other) {
        this->foo = std::move(other.foo);
        *data_ostr << 4;
        return *this;
    }
    QString foo;
};
std::ostream* Data::data_ostr = nullptr;

} // Result_test

void Test::Result_test()
{
    using Result = Result<Result_test::Data>;
    {
        std::ostringstream sstr;
        Result::Data::data_ostr = &sstr;
        const Result res = Result::error("error_description");
        QVERIFY(res.errorText() == "error_description");
        QVERIFY(!res.valid());
        QCOMPARE(sstr.str().c_str(), "02");
    }
    {
        std::ostringstream sstr;
        Result::Data::data_ostr = &sstr;
        Result::Data data;
        data.foo = "FooData";
        const Result res = Result::ok(std::move(data));
        QVERIFY(res.valid());
        QVERIFY(res.get().foo == "FooData");
        QCOMPARE(sstr.str().c_str(), "0042");
    }
}

void Test::UnitSystem_test()
{
    const UnitSystem::Schema schemaSI = UnitSystem::SI;
    using TranslateResult_Test =
        std::pair<UnitSystem::TranslateResult, UnitSystem::TranslateResult>;
    const TranslateResult_Test array[] = {
        { UnitSystem::translate(schemaSI, 80 * Quantity_Millimeter), { 80., "mm", 1. } },
        { UnitSystem::translate(schemaSI, 8 * Quantity_Centimeter),  { 80., "mm", 1. } },
        { UnitSystem::translate(schemaSI, 8 * Quantity_Meter),  { 8000., "mm", 1. } },
        { UnitSystem::translate(schemaSI, 0.5 * Quantity_SquaredCentimer), { 50., "mm²", 1. } }
    };
    for (const TranslateResult_Test& test : array) {
        QCOMPARE(test.first, test.second);
    }
}

void Test::LibTree_test()
{
    const TreeNodeId nullptrId = 0;
    Tree<std::string> tree;
    TreeNodeId n0 = tree.appendChild(0, "0");
    TreeNodeId n0_1 = tree.appendChild(n0, "0-1");
    TreeNodeId n0_2 = tree.appendChild(n0, "0-2");
    TreeNodeId n0_1_1 = tree.appendChild(n0_1, "0-1-1");
    TreeNodeId n0_1_2 = tree.appendChild(n0_1, "0-1-2");

    QCOMPARE(tree.nodeParent(n0_1), n0);
    QCOMPARE(tree.nodeParent(n0_2), n0);
    QCOMPARE(tree.nodeParent(n0_1_1), n0_1);
    QCOMPARE(tree.nodeParent(n0_1_2), n0_1);
    QCOMPARE(tree.nodeChildFirst(n0_1), n0_1_1);
    QCOMPARE(tree.nodeChildLast(n0_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingNext(n0_1_1), n0_1_2);
    QCOMPARE(tree.nodeSiblingPrevious(n0_1_2), n0_1_1);
    QCOMPARE(tree.nodeSiblingNext(n0_1_2), nullptrId);
}

} // namespace Mayo

