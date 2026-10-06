/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_promise.h"

namespace Mayo {

QuickJsPromise::QuickJsPromise(JSContext* context)
    : m_context(context)
{
    JSValue resolvingFunctions[2] = { JS_UNDEFINED, JS_UNDEFINED };

    const JSValue promise = JS_NewPromiseCapability(context, resolvingFunctions);
    if (JS_IsException(promise))
        return;

    m_resolve = QuickJsValue{context, resolvingFunctions[0]};
    m_reject = QuickJsValue{context, resolvingFunctions[1]};
    m_promise = QuickJsValue{context, promise};
}

QuickJsValue QuickJsPromise::get() const
{
    return m_promise.dup();
}

JSPromiseStateEnum QuickJsPromise::state() const
{
    return m_promise.promiseState();
}

QuickJsValue QuickJsPromise::result() const
{
    return m_promise.promiseResult();
}

bool QuickJsPromise::resolve(JSValueConst value)
{
    if (!m_resolve)
        return false;

    const auto arg = QuickJsValue::dup(m_context, value);
    const auto result = m_resolve.call(JS_UNDEFINED, { arg.get() });
    return !result.isException();
}

bool QuickJsPromise::reject(JSValueConst reason)
{
    if (!m_reject)
        return false;

    const auto arg = QuickJsValue::dup(m_context, reason);
    const auto result = m_reject.call(JS_UNDEFINED, { arg.get() });
    return !result.isException();
}

bool QuickJsPromise::reject(std::string_view strMessage)
{
    if (!m_reject)
        return false;

    const auto reason = QuickJsValue::newError(m_context, strMessage);
    if (reason.isException())
        return false;

    return this->reject(reason.get());
}

} // namespace Mayo

