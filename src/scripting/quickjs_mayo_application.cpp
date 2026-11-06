/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_mayo_application.h"

#include "quickjs_binding_helper.h"
#include "quickjs_context.h"

#include <exception>
#include <memory>
#include <utility>

namespace Mayo {

QuickJsMayoApplication::QuickJsMayoApplication(ApplicationPtr application, bool graphics)
    : m_application(std::move(application)),
      m_graphics(graphics)
{}

QuickJsValue QuickJsMayoApplication::create(JSContext* context, ApplicationPtr application, bool graphics)
{
    if (application.IsNull()) {
        return QuickJsContext::throwError(
            context, "Cannot create a Mayo.Application from a null Application"
        );
    }

    const JSClassID id = Binding::classId(context);
    if (id == JS_INVALID_CLASS_ID)
        return QuickJsContext::throwInternalError(context, "MayoApplication class is not registered");

    auto opaque = std::make_unique<QuickJsMayoApplication>(std::move(application), graphics);

    auto jsObject = QuickJsValue::newObjectClass(context, id);
    if (jsObject.isException())
        return jsObject;

    auto jsGraphics = QuickJsValue::newBool(context, opaque->graphics());
    if (!jsObject.defineProperty("graphics", std::move(jsGraphics), JS_PROP_ENUMERABLE))
        return { context, JS_EXCEPTION };

    jsObject.setOpaquePtr(opaque.release());
    return jsObject;
}

JSValue QuickJsMayoApplication::constructor(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    bool graphics = false;
    if (argc > 0 && !JS_IsUndefined(argv[0])) {
        if (!JS_IsObject(argv[0]) || JS_IsNull(argv[0]))
            return JS_ThrowTypeError(context, "Application options must be an object");

        auto jsGraphics = QuickJsValue::getProperty(context, argv[0], "graphics");
        if (jsGraphics.isException())
            return jsGraphics.release();

        if (!jsGraphics.isUndefined()) {
            if (!jsGraphics.isBool())
                return JS_ThrowTypeError(context, "Application option 'graphics' must be a boolean");

            graphics = jsGraphics.toBool().value_or(false);
        }
    }

    try {
        return QuickJsMayoApplication::create(context, makeOccHandle<Application>(), graphics).release();
    }
    catch (const std::exception& ex) {
        return JS_ThrowInternalError(context, "%s", ex.what());
    }
    catch (...) {
        return JS_ThrowInternalError(context, "Failed to create Mayo.Application");
    }
}

JSValue QuickJsMayoApplication::documentCount(JSContext* context, JSValueConst thisValue)
{
    auto opaque = Binding::getOpaque(context, thisValue);
    if (!opaque)
        return JS_EXCEPTION;

    if (opaque->m_application.IsNull())
        return JS_ThrowInternalError(context, "Application is no longer available");

    return JS_NewInt32(context, opaque->m_application->documentCount());
}

QuickJsValue QuickJsMayoApplication::createPrototype(JSContext* context)
{
    auto prototype = QuickJsValue::newObject(context);
    if (prototype.isException())
        return prototype;

    const JSCFunctionListEntry prototypeProps[] = {
        quickjs_createPropertyGetter("documentCount", &QuickJsMayoApplication::documentCount)
    };
    if (JS_SetPropertyFunctionList(context, prototype.get(), prototypeProps, 1) < 0)
        return { context, JS_EXCEPTION };

    const JSClassID id = Binding::classId(context);
    if (id == JS_INVALID_CLASS_ID)
        return QuickJsContext::throwInternalError(context, "MayoApplication class is not registered");

    JS_SetClassProto(context, id, prototype.dup().release());
    return prototype;
}

QuickJsValue QuickJsMayoApplication::createConstructor(JSContext* context, const QuickJsValue& prototype)
{
    auto ctor = QuickJsValue::newFunction(
        context, &QuickJsMayoApplication::constructor, "Application", JS_CFUNC_constructor, 1
    );
    if (ctor.isException())
        return ctor;

    JS_SetConstructor(context, ctor.get(), prototype.get());
    return ctor;
}

} // namespace Mayo
