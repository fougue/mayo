/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_mayo_application.h"

#include "quickjs_binding_helper.h"
#include "quickjs_context.h"
#include "quickjs_mayo_document.h"

#include "../base/ithread_dispatch.h"

#include <exception>
#include <memory>
#include <utility>

namespace Mayo {

QuickJsMayoApplication::QuickJsMayoApplication(
        ApplicationPtr application, bool graphics, IThreadDispatch* threadDispatch
    )
    : m_threadDispatch(threadDispatch),
      m_application(std::move(application)),
      m_graphics(graphics)
{
}

QuickJsValue QuickJsMayoApplication::create(
        JSContext* context, ApplicationPtr application, bool graphics, IThreadDispatch* threadDispatch
    )
{
    if (application.IsNull()) {
        return QuickJsContext::throwError(
            context, "Cannot create a Mayo.Application from a null Application"
        );
    }

    const JSClassID id = Binding::classId(context);
    if (id == JS_INVALID_CLASS_ID)
        return QuickJsContext::throwInternalError(context, "MayoApplication class is not registered");

    auto opaque = std::make_unique<QuickJsMayoApplication>(std::move(application), graphics, threadDispatch);

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

    auto createApp = [graphics](JSContext* ctx){
        return create(ctx, makeOccHandle<Application>(), graphics).release();
    };
    return QuickJsContext::noCppThrowExec(context, createApp, "Failed to create mayo.Application");
}

JSValue QuickJsMayoApplication::documentCount(JSContext* context, JSValueConst thisValue)
{
    auto opaque = getOpaqueOrThrow(context, thisValue);
    if (!opaque)
        return JS_EXCEPTION;

    auto getDocCount = [opaque](JSContext* ctx) {
        const int count = IThreadDispatch::invokeIfSet(opaque->m_threadDispatch, [=]{
            return opaque->m_application->documentCount();
        });
        return JS_NewInt32(ctx, count);
    };
    return QuickJsContext::noCppThrowExec(context, getDocCount, "Failed to read document count");
}

JSValue QuickJsMayoApplication::newDocument(
        JSContext* context, JSValueConst thisValue, int, JSValueConst*
    )
{
    auto opaque = getOpaqueOrThrow(context, thisValue);
    if (!opaque)
        return JS_EXCEPTION;

    auto newDoc = [opaque](JSContext* ctx) {
        auto threadDispatch = opaque->m_threadDispatch;
        DocumentPtr doc = IThreadDispatch::invokeIfSet(threadDispatch, [=]{
            return opaque->m_application->newDocument();
        });
        return doc.IsNull()
            ? JS_ThrowInternalError(ctx, "Failed to create document")
            : QuickJsMayoDocument::create(ctx, std::move(doc), threadDispatch).release()
        ;
    };
    return QuickJsContext::noCppThrowExec(context, newDoc, "Failed to create document");
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

    if (!prototype.setFunction("newDocument", &QuickJsMayoApplication::newDocument))
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

QuickJsMayoApplication* QuickJsMayoApplication::getOpaqueOrThrow(JSContext* context, JSValueConst thisValue)
{
    auto opaque = Binding::getOpaque(context, thisValue);
    if (!opaque)
        return {}; // JS_GetOpaque2() already set exception

    if (opaque->m_application.IsNull()) {
        JS_ThrowInternalError(context, "Application is no longer available");
        return {};
    }

    return opaque;
}

} // namespace Mayo
