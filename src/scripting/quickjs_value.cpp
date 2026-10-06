/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_value.h"
#include "quickjs_context.h"

namespace Mayo {

QuickJsValue::QuickJsValue(JSContext* context, JSValue value)
    : Base(value, QuickJsValueTraits{context})
{
}

JSContext* QuickJsValue::context() const
{
    return this->traits().context;
}

void* QuickJsValue::ptr() const
{
    return JS_VALUE_GET_PTR(this->get());
}

QuickJsValue QuickJsValue::dup() const
{
    if (!this->context())
        return QuickJsValue{};

    return QuickJsValue::dup(this->context(), this->get());
}

QuickJsValue QuickJsValue::call(JSValueConst thisVal, std::initializer_list<JSValueConst> args) const
{
    const int argCount = static_cast<int>(args.size());
    JSValueConst* argPtr = const_cast<JSValueConst*>(args.begin());
    return { this->context(), JS_Call(this->context(), this->get(), thisVal, argCount, argPtr) };
}

bool QuickJsValue::isUndefined() const
{
    return JS_IsUndefined(this->get());
}

bool QuickJsValue::isException() const
{
    return JS_IsException(this->get());
}

bool QuickJsValue::isError() const
{
    return JS_IsError(this->get());
}

JSPromiseStateEnum QuickJsValue::promiseState() const
{
    return JS_PromiseState(this->context(), this->get());
}

QuickJsValue QuickJsValue::promiseResult() const
{
    return { this->context(), JS_PromiseResult(this->context(), this->get()) };
}

QuickJsValue QuickJsValue::getProperty(const char* name) const
{
    return { this->context(), JS_GetPropertyStr(this->context(), this->get(), name) };
}

bool QuickJsValue::setProperty(const char* name, QuickJsValue value)
{
    return QuickJsValue::setProperty(name, value.release());
}

bool QuickJsValue::setProperty(const char* name, JSValue value)
{
    return JS_SetPropertyStr(this->context(), this->get(), name, value) >= 0;
}

std::optional<int32_t> QuickJsValue::toInt32() const
{
    if (!this->context())
        return std::nullopt;

    int32_t value = 0;
    if (JS_ToInt32(this->context(), &value, this->get()) < 0)
        return std::nullopt;

    return value;
}

std::optional<std::string> QuickJsValue::toStdString() const
{
    if (!this->context())
        return std::nullopt;

    size_t length = 0;
    const char* str = JS_ToCStringLen(this->context(), &length, this->get());
    if (!str)
        return std::nullopt;

    const auto result = std::string{str, length};
    JS_FreeCString(this->context(), str);
    return result;
}

QuickJsValue QuickJsValue::dup(JSContext* context, JSValueConst value)
{
    return { context, JS_DupValue(context, value) };
}

QuickJsValue QuickJsValue::newError(JSContext* context, std::string_view message)
{
    QuickJsValue jsError{context, JS_NewError(context)};
    if (jsError.isException())
        return jsError;

    QuickJsValue jsMessage = QuickJsValue::newString(context, message);
    if (jsMessage.isException())
        return jsMessage;

    if (!jsError.setProperty("message", std::move(jsMessage)))
        return QuickJsContext::takeException(context);

    return jsError;
}

QuickJsValue QuickJsValue::newString(JSContext* context, std::string_view str)
{
    return { context, JS_NewStringLen(context, str.data(), str.size()) };
}

QuickJsValue QuickJsValue::newInt32(JSContext* context, int32_t val)
{
    return { context, JS_NewInt32(context, val) };
}

QuickJsValue QuickJsValue::newObject(JSContext* context)
{
    return { context, JS_NewObject(context) };
}

QuickJsValue QuickJsValue::newFunction(
        JSContext* context, JSCFunction* func, const char* name, int length
    )
{
    return { context, JS_NewCFunction(context, func, name, length) };
}

} // namespace Mayo
