/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "quickjs_base_handle.h"
#include "quickjs_value.h"

#include <quickjs.h>
#include <exception>
#include <string_view>
#include <utility>

namespace Mayo {

struct QuickJsContextTraits {
    static constexpr JSContext* empty() noexcept { return nullptr; }
    bool isEmpty(JSContext* context) const noexcept { return context == nullptr; }
    void destroy(JSContext* context) const { JS_FreeContext(context); }
};

// Owning RAII wrapper around a QuickJS context
// All operations and destruction must occur on the thread that owns the context
class QuickJsContext : public QuickJsBaseHandle<JSContext*, QuickJsContextTraits> {
private:
    using Base = QuickJsBaseHandle<JSContext*, QuickJsContextTraits>;

public:
    using Base::Base;

    QuickJsValue eval(std::string_view script, const FilePath& filename, int evalFlags) const;
    QuickJsValue evalFunction(QuickJsValue funObj) const;

    QuickJsValue takeException() const;
    static QuickJsValue takeException(JSContext* context);

    static QuickJsValue throwInternalError(JSContext* context, const char* message);

    static QuickJsValue throwError(JSContext* context, std::string_view message);
    QuickJsValue throwError(std::string_view message) const;

    static QuickJsValue getGlobalObject(JSContext* context);

    QuickJsValue getModuleNamespace(JSModuleDef* module) const;
    QuickJsValue getModuleDefaultExport(JSModuleDef* module) const;

    template<typename Function>
    static JSValue noCppThrowExec(JSContext* context, Function&& fn, const char* catchAllErrorMsg = "");
};



// --
// -- Implementation
// --

template<typename Function>
JSValue QuickJsContext::noCppThrowExec(JSContext* context, Function&& fn, const char* catchAllErrorMsg)
{
    try {
        return std::forward<Function>(fn)(context);
    }
    catch (const std::exception& err) {
        return JS_ThrowInternalError(context, "%s", err.what());
    }
    catch (...) {
        return JS_ThrowInternalError(context, "%s", catchAllErrorMsg);
    }
}

} // namespace Mayo
