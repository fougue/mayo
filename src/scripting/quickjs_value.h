/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_base_handle.h"

#include <quickjs.h>

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>

namespace Mayo {

struct QuickJsValueTraits {
    JSContext* context = nullptr;

    static JSValue empty() noexcept { return JS_UNDEFINED; }
    bool isEmpty(JSValue value) const noexcept { return !this->context || JS_IsUndefined(value); }
    void destroy(JSValue value) const
    {
        if (this->context)
            JS_FreeValue(this->context, value);
    }
};

class QuickJsValue : public QuickJsBaseHandle<JSValue, QuickJsValueTraits> {
private:
    using Base = QuickJsBaseHandle<JSValue, QuickJsValueTraits>;

public:
    QuickJsValue() = default;
    QuickJsValue(JSContext* context, JSValue value);

    QuickJsValue(QuickJsValue&&) noexcept = default;
    QuickJsValue& operator=(QuickJsValue&&) noexcept = default;

    JSContext* context() const;

    void* ptr() const;

    QuickJsValue dup() const;

    QuickJsValue call(JSValueConst thisVal, std::initializer_list<JSValueConst> args = {}) const;

    bool isUndefined() const;
    bool isException() const;
    bool isError() const;

    JSPromiseStateEnum promiseState() const;
    QuickJsValue promiseResult() const;

    QuickJsValue getProperty(const char* name) const;

    std::optional<int32_t> toInt32() const;
    std::optional<std::string> toStdString() const;

    static QuickJsValue dup(JSContext* context, JSValueConst value);
    static QuickJsValue newError(JSContext* context);
    static QuickJsValue newString(JSContext* context, std::string_view str);
    static QuickJsValue newInt32(JSContext* context, int32_t val);
};

} // namespace Mayo
