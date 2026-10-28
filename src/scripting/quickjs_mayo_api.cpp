/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_mayo_api.h"

#include "quickjs_class_registry.h"
#include "quickjs_context.h"
#include "quickjs_script_evaluation.h"
#include "quickjs_script_engine.h"
#include "script_host_environment.h"

#include "../base/application.h"
#include "../base/occ_handle.h"

#include <exception>
#include <memory>

namespace Mayo {

namespace {

class QuickJsApplication {
public:
    static constexpr const char* className = "MayoApplication";

    QuickJsApplication(ApplicationPtr application, bool graphics)
        : m_application(std::move(application)),
          m_graphics(graphics)
    {}

    static bool registerClass(QuickJsClassRegistry* registry)
    {
        if (registry->findClassId(className) != JS_INVALID_CLASS_ID)
            return true;

        return registry->newClass(className, &QuickJsApplication::finalizer);
    }

    static QuickJsValue create(JSContext* context, ApplicationPtr application, bool graphics)
    {
        if (application.IsNull()) {
            return QuickJsContext::throwError(
                context, "Cannot create a Mayo.Application from a null Application"
            );
        }

        const JSClassID classId = QuickJsApplication::classId(context);
        if (classId == JS_INVALID_CLASS_ID)
            return QuickJsContext::throwInternalError(context, "MayoApplication class is not registered");

        auto opaque = std::make_unique<QuickJsApplication>(std::move(application), graphics);

        auto jsObject = QuickJsValue::newObjectClass(context, classId);
        if (jsObject.isException())
            return jsObject;

        auto jsGraphics = QuickJsValue::newBool(context, opaque->graphics());
        if (!jsObject.defineProperty("graphics", std::move(jsGraphics), JS_PROP_ENUMERABLE))
            return { context, JS_EXCEPTION };

        jsObject.setOpaquePtr(opaque.release());
        return jsObject;
    }

    static void finalizer(JSRuntime*, JSValue value)
    {
        const JSClassID classId = JS_GetClassID(value);
        if (classId != JS_INVALID_CLASS_ID)
            delete static_cast<QuickJsApplication*>(JS_GetOpaque(value, classId));
    }

    static JSValue constructor(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
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
            return QuickJsApplication::create(context, makeOccHandle<Application>(), graphics).release();
        }
        catch (const std::exception& ex) {
            return JS_ThrowInternalError(context, "%s", ex.what());
        }
        catch (...) {
            return JS_ThrowInternalError(context, "Failed to create Mayo.Application");
        }
    }

    static JSValue documentCount(JSContext* context, JSValueConst thisValue)
    {
        QuickJsApplication* opaque = QuickJsApplication::getOpaque(context, thisValue);
        if (!opaque)
            return JS_EXCEPTION;

        if (opaque->m_application.IsNull())
            return JS_ThrowInternalError(context, "Application is no longer available");

        return JS_NewInt32(context, opaque->m_application->documentCount());
    }

    static QuickJsValue createPrototype(JSContext* context)
    {
        auto prototype = QuickJsValue::newObject(context);
        if (prototype.isException())
            return prototype;

        JSCFunctionListEntry prototypeProperties[1]{};
        prototypeProperties[0].name = "documentCount";
        prototypeProperties[0].prop_flags = JS_PROP_CONFIGURABLE;
        prototypeProperties[0].def_type = JS_DEF_CGETSET;
        prototypeProperties[0].u.getset.get.getter = &QuickJsApplication::documentCount;
        if (JS_SetPropertyFunctionList(context, prototype.get(), prototypeProperties, 1) < 0)
            return { context, JS_EXCEPTION };

        const JSClassID classId = QuickJsApplication::classId(context);
        if (classId == JS_INVALID_CLASS_ID)
            return QuickJsContext::throwInternalError(context, "MayoApplication class is not registered");

        JS_SetClassProto(context, classId, prototype.dup().release());
        return prototype;
    }

    static QuickJsValue createConstructor(JSContext* context, const QuickJsValue& prototype)
    {
        auto ctor = QuickJsValue::newFunction(
            context, &QuickJsApplication::constructor, "Application", JS_CFUNC_constructor, 1
        );
        if (ctor.isException())
            return ctor;

        JS_SetConstructor(context, ctor.get(), prototype.get());
        return ctor;
    }

private:
    static JSClassID classId(JSContext* context)
    {
        const auto registry = QuickJsScriptEngine::getClassRegistry(JS_GetRuntime(context));
        return registry ? registry->findClassId(className) : JS_INVALID_CLASS_ID;
    }

    static QuickJsApplication* getOpaque(JSContext* context, JSValueConst thisValue)
    {
        const JSClassID id = QuickJsApplication::classId(context);
        return id != JS_INVALID_CLASS_ID
            ? static_cast<QuickJsApplication*>(JS_GetOpaque2(context, thisValue, id))
            : nullptr;
    }

    ApplicationPtr m_application;
    bool m_graphics{false};

    bool graphics() const { return m_graphics; }
};

QuickJsValue installMayoApiImpl(
        QuickJsScriptEvaluation& evaluation, const ScriptHostEnvironment& hostEnv
    )
{
    JSContext* context = evaluation.context();
    auto registry = QuickJsScriptEngine::getClassRegistry(JS_GetRuntime(context));
    if (!registry)
        return QuickJsContext::throwInternalError(context, "No QuickJS class registry is available");

    // -- Application bindings
    if (!QuickJsApplication::registerClass(registry))
        return QuickJsContext::throwInternalError(context, "Failed to register MayoApplication");

    QuickJsValue appPrototype = QuickJsApplication::createPrototype(context);
    if (appPrototype.isException())
        return appPrototype;

    QuickJsValue appCtor = QuickJsApplication::createConstructor(context, appPrototype);
    if (appCtor.isException())
        return appCtor;

    // -- "mayo" object
    QuickJsValue mayo = QuickJsValue::newObject(context);
    if (mayo.isException())
        return mayo;

    if (!mayo.setProperty("Application", std::move(appCtor)))
        return { context, JS_EXCEPTION };

    // The host enables graphics post-processing by providing this callback
    const bool hostHasGraphics = static_cast<bool>(hostEnv.ioEntityImportPostProcess);
    QuickJsValue hostApplication = QuickJsApplication::create(
        context, hostEnv.application, hostHasGraphics
    );
    if (hostApplication.isException())
        return hostApplication;

    if (!mayo.setProperty("app", std::move(hostApplication)))
        return { context, JS_EXCEPTION };

    QuickJsValue global = QuickJsContext::getGlobalObject(context);
    if (global.isException())
        return global;

    if (!global.setProperty("mayo", std::move(mayo)))
        return { context, JS_EXCEPTION };

    return {};
}

} // namespace

QuickJsValue quickjs_installMayoApi(
        QuickJsScriptEvaluation& evaluation, const ScriptHostEnvironment& hostEnv
    )
{
    try {
        return installMayoApiImpl(evaluation, hostEnv);
    }
    catch (const std::exception& ex) {
        return QuickJsContext::throwInternalError(evaluation.context(), ex.what());
    }
    catch (...) {
        return QuickJsContext::throwInternalError(evaluation.context(), "Failed to install Mayo API");
    }
}

} // namespace Mayo
