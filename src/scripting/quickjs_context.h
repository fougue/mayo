/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/filepath.h"
#include "quickjs_base_handle.h"
#include "quickjs_value.h"

#include <quickjs.h>
#include <string_view>

namespace Mayo {

struct QuickJsRuntimeTraits {
    static constexpr JSRuntime* empty() noexcept { return nullptr; }
    bool isEmpty(JSRuntime* runtime) const noexcept { return runtime == nullptr; }
    void destroy(JSRuntime* runtime) const { JS_FreeRuntime(runtime); }
};

struct QuickJsContextTraits {
    static constexpr JSContext* empty() noexcept { return nullptr; }
    bool isEmpty(JSContext* context) const noexcept { return context == nullptr; }
    void destroy(JSContext* context) const { JS_FreeContext(context); }
};

using QuickJsRuntime = QuickJsBaseHandle<JSRuntime*, QuickJsRuntimeTraits>;

class QuickJsContext : public QuickJsBaseHandle<JSContext*, QuickJsContextTraits> {
private:
    using Base = QuickJsBaseHandle<JSContext*, QuickJsContextTraits>;
public:
    using Base::Base;

    QuickJsValue eval(std::string_view script, const FilePath& filename, int evalFlags) const;
    QuickJsValue evalFunction(QuickJsValue funObj) const;

    QuickJsValue takeException() const;
    static QuickJsValue takeException(JSContext* context);

    QuickJsValue getModuleNamespace(JSModuleDef* module) const;
};

} // namespace Mayo
