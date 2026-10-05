/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_scripting.h"

#include "../scripting/quickjs_context.h"
#include "../scripting/quickjs_promise.h"

namespace Mayo {

namespace {

struct QuickJsPromiseTestHelper {
    QuickJsRuntime runtime{JS_NewRuntime()};
    QuickJsContext context{JS_NewContext(runtime.get())};
    QuickJsPromise promise{context.get()};
};

} // namespace

void TestScripting::QuickJsPromise_promise_test()
{
    QuickJsPromiseTestHelper helper;
    QCOMPARE(helper.promise.state(), JS_PROMISE_PENDING);
}

void TestScripting::QuickJsPromise_resolve_test()
{
    QuickJsPromiseTestHelper helper;

    auto value = QuickJsValue::newInt32(helper.context.get(), 42);
    QVERIFY(helper.promise.resolve(value.get()));

    QCOMPARE(helper.promise.state(), JS_PROMISE_FULFILLED);

    const QuickJsValue result = helper.promise.result();
    QCOMPARE(result.toInt32().value_or(0), 42);
}

void TestScripting::QuickJsPromise_resolveUndefined_test()
{
    QuickJsPromiseTestHelper helper;
    QVERIFY(helper.promise.resolve(JS_UNDEFINED));
    QCOMPARE(helper.promise.state(), JS_PROMISE_FULFILLED);
    QVERIFY(helper.promise.result().isUndefined());
}

void TestScripting::QuickJsPromise_rejectValue_test()
{
    QuickJsPromiseTestHelper helper;

    const auto reason = QuickJsValue::newString(helper.context.get(), "failure");
    QVERIFY(helper.promise.reject(reason.get()));
    QCOMPARE(helper.promise.state(), JS_PROMISE_REJECTED);

    const auto result = helper.promise.result();
    QCOMPARE(result.toStdString().value_or(std::string{}), "failure");
}

void TestScripting::QuickJsPromise_rejectMessage_test()
{
    QuickJsPromiseTestHelper helper;

    QVERIFY(helper.promise.reject("Something went wrong"));
    QCOMPARE(helper.promise.state(), JS_PROMISE_REJECTED);

    const auto reason = helper.promise.result();
    QVERIFY(reason.isError());

    const auto message = reason.getProperty("message");
    QVERIFY(!message.isException());
    QCOMPARE(message.toStdString().value_or(std::string{}), "Something went wrong");
}

void TestScripting::QuickJsPromise_resolveThenReject_test()
{
    QuickJsPromiseTestHelper helper;

    const auto value = QuickJsValue::newInt32(helper.context.get(), 42);
    QVERIFY(helper.promise.resolve(value.get()));

    QVERIFY(helper.promise.reject("This must be ignored"));
    QCOMPARE(helper.promise.state(), JS_PROMISE_FULFILLED);

    const auto result = helper.promise.result();
    QCOMPARE(result.toInt32().value_or(0), 42);
}

void TestScripting::QuickJsPromise_rejectThenResolve_test()
{
    QuickJsPromiseTestHelper helper;

    QVERIFY(helper.promise.reject("failure"));
    QCOMPARE(helper.promise.state(), JS_PROMISE_REJECTED);

    QVERIFY(helper.promise.resolve(JS_UNDEFINED));
    QCOMPARE(helper.promise.state(), JS_PROMISE_REJECTED);

    const auto reason = helper.promise.result();
    QVERIFY(reason.isError());
}

void TestScripting::QuickJsPromise_duplicatedPromiseReference_test()
{
    QuickJsPromiseTestHelper helper;

    auto first = helper.promise.get();
    auto second = helper.promise.get();

    QVERIFY(!first.isUndefined());
    QVERIFY(!second.isUndefined());

    // Releasing one reference must not invalidate the other one
    first.reset();

    QCOMPARE(JS_PromiseState(helper.context.get(), second.get()), JS_PROMISE_PENDING);

    const auto value = QuickJsValue::newInt32(helper.context.get(), 123);
    QVERIFY(helper.promise.resolve(value.get()));
    QCOMPARE(JS_PromiseState(helper.context.get(), second.get()), JS_PROMISE_FULFILLED);
}

} // namespace Mayo