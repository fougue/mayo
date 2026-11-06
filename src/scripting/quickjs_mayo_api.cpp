/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_mayo_api.h"

#include "quickjs_class_registry.h"
#include "quickjs_context.h"
#include "quickjs_mayo_application.h"
#include "quickjs_mayo_document.h"
#include "quickjs_script_evaluation.h"
#include "quickjs_script_engine.h"
#include "script_host_environment.h"

#include "quickjs_binding_helper.h"

#include <exception>
#include <utility>

namespace Mayo {

namespace {

QuickJsValue installMayoApiImpl(
        QuickJsScriptEvaluation& evaluation, const ScriptHostEnvironment& hostEnv
    )
{
    JSContext* context = evaluation.context();
    auto registry = QuickJsScriptEngine::getClassRegistry(JS_GetRuntime(context));
    if (!registry)
        return QuickJsContext::throwInternalError(context, "No QuickJS class registry is available");

    // -- Application bindings
    if (!QuickJsMayoApplication::Binding::registerClass(registry))
        return QuickJsContext::throwInternalError(context, "Failed to register MayoApplication");

    if (!QuickJsMayoDocument::Binding::registerClass(registry))
        return QuickJsContext::throwInternalError(context, "Failed to register MayoDocument");

    QuickJsValue appPrototype = QuickJsMayoApplication::createPrototype(context);
    if (appPrototype.isException())
        return appPrototype;

    QuickJsValue documentPrototype = QuickJsMayoDocument::createPrototype(context);
    if (documentPrototype.isException())
        return documentPrototype;

    QuickJsValue appCtor = QuickJsMayoApplication::createConstructor(context, appPrototype);
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
    QuickJsValue hostApplication = QuickJsMayoApplication::create(
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
    catch (const std::exception& err) {
        return QuickJsContext::throwInternalError(evaluation.context(), err.what());
    }
    catch (...) {
        return QuickJsContext::throwInternalError(evaluation.context(), "Failed to install Mayo API");
    }
}

} // namespace Mayo
