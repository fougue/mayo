/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"
#include "quickjs_binding_helper.h"

#include "../base/application.h"

namespace Mayo {

class QuickJsClassRegistry;

class QuickJsMayoApplication {
public:
    using Binding = QuickJsBindingHelper<QuickJsMayoApplication>;

    static constexpr const char* className = "MayoApplication";

    QuickJsMayoApplication(ApplicationPtr application, bool graphics);

    static QuickJsValue create(JSContext* context, ApplicationPtr application, bool graphics);
    static QuickJsValue createPrototype(JSContext* context);
    static QuickJsValue createConstructor(JSContext* context, const QuickJsValue& prototype);

private:
    static JSValue constructor(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
    static JSValue documentCount(JSContext* context, JSValueConst thisValue);

    ApplicationPtr m_application;
    bool m_graphics{false};

    bool graphics() const { return m_graphics; }
};

} // namespace Mayo
