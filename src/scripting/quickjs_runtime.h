/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_base_handle.h"

#include <quickjs.h>

#include <string_view>
#include <unordered_map>

namespace Mayo {

struct QuickJsRuntimeTraits {
    static constexpr JSRuntime* empty() noexcept { return nullptr; }
    bool isEmpty(JSRuntime* runtime) const noexcept { return runtime == nullptr; }
    void destroy(JSRuntime* runtime) const { JS_FreeRuntime(runtime); }
};

// Owning RAII wrapper around a QuickJS runtime
// The runtime must outlive every context and JavaScript value created from it
class QuickJsRuntime : public QuickJsBaseHandle<JSRuntime*, QuickJsRuntimeTraits> {
private:
    using Base = QuickJsBaseHandle<JSRuntime*, QuickJsRuntimeTraits>;

public:
    using Base::Base;
};

} // namespace Mayo
