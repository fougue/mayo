/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_class_registry.h"
#include "quickjs_script_engine.h"

#include <quickjs.h>

namespace Mayo {

// Provides common QuickJS class registration and opaque-pointer handling for a C++ binding type
// The registered finalizer deletes the binding object stored in the JavaScript object's opaque slot
template<typename T>
class QuickJsBindingHelper {
public:
    static void finalizer(JSRuntime*, JSValue value)
    {
        const JSClassID id = JS_GetClassID(value);
        if (id != JS_INVALID_CLASS_ID)
            delete static_cast<T*>(JS_GetOpaque(value, id));
    }

    static bool registerClass(QuickJsClassRegistry* registry)
    {
        if (registry->findClassId(T::className) != JS_INVALID_CLASS_ID)
            return true;

        return registry->newClass(T::className, &finalizer);
    }

    static JSClassID classId(JSContext* context)
    {
        const auto registry = QuickJsScriptEngine::getClassRegistry(JS_GetRuntime(context));
        return registry ? registry->findClassId(T::className) : JS_INVALID_CLASS_ID;
    }

    static T* getOpaque(JSContext* context, JSValueConst thisValue)
    {
        const JSClassID id = classId(context);
        return id != JS_INVALID_CLASS_ID
            ? static_cast<T*>(JS_GetOpaque2(context, thisValue, id))
            : nullptr
        ;
    }
};

inline JSCFunctionListEntry quickjs_createPropertyGetter(
        const char* name, JSValue (*fn)(JSContext* ctx, JSValueConst thisVal)
    )
{
    JSCFunctionListEntry entry{};
    entry.name = name;
    entry.prop_flags = JS_PROP_CONFIGURABLE;
    entry.def_type = JS_DEF_CGETSET;
    entry.u.getset.get.getter = fn;
    return entry;
}

} // namespace Mayo
