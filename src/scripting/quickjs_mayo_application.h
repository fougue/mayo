/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"
#include "quickjs_binding_helper.h"

#include "../base/application.h"

namespace Mayo {

class IThreadDispatch;
class QuickJsClassRegistry;

// QuickJS binding for a Mayo Application
// The wrapper owns an ApplicationPtr and optionally dispatches its operations to the application's thread
class QuickJsMayoApplication {
public:
    using Binding = QuickJsBindingHelper<QuickJsMayoApplication>;

    static constexpr const char* className = "MayoApplication";

    QuickJsMayoApplication(ApplicationPtr application, bool graphics, IThreadDispatch* threadDispatch);

    static QuickJsValue create(
        JSContext* context,
        ApplicationPtr application,
        bool graphics,
        IThreadDispatch* threadDispatch = nullptr
    );

    static QuickJsValue createPrototype(JSContext* context);
    static QuickJsValue createConstructor(JSContext* context, const QuickJsValue& prototype);

private:
    static JSValue constructor(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
    static JSValue documentCount(JSContext* context, JSValueConst thisValue);
    static JSValue newDocument(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);

    static QuickJsMayoApplication* getOpaqueOrThrow(JSContext* context, JSValueConst thisValue);

    IThreadDispatch* m_threadDispatch{nullptr}; // Non-owning
    ApplicationPtr m_application;
    bool m_graphics{false};

    bool graphics() const { return m_graphics; }
};

} // namespace Mayo
