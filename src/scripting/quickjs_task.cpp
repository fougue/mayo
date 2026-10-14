/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_task.h"

#include "quickjs_runtime.h"
#include "quickjs_script_engine.h"
#include "quickjs_script_evaluation.h"

#include <memory>

namespace Mayo {

namespace {

struct TaskHandle {
    TaskHandle(QuickJsScriptEvaluation* eval, TaskId id)
        : evaluation(eval), taskId(id)
    {}

    QuickJsScriptEvaluation* evaluation{nullptr};
    TaskId taskId{TaskId_null};

    static TaskHandle* getOpaquePtrOrThrow(JSContext* context, JSValueConst thisValue)
    {
        auto classId = QuickJsScriptEngine::findClassId(JS_GetRuntime(context), QuickJsTask::className);
        return classId != JS_INVALID_CLASS_ID
            ? static_cast<TaskHandle*>(JS_GetOpaque2(context, thisValue, classId))
            : nullptr;
    }
};

} // namespace

QuickJsValue QuickJsTask::create(QuickJsScriptEvaluation& evaluation, TaskId taskId)
{
    JSContext* context = evaluation.context();

    auto jsThrowError = [=](const char* msg) {
        return QuickJsValue{ context, JS_ThrowInternalError(context, msg) };
    };

    auto classId = QuickJsScriptEngine::findClassId(JS_GetRuntime(context), className);
    if (classId == JS_INVALID_CLASS_ID)
        return jsThrowError("No class ID defined for QuickJsTask");

    QuickJsValue object{context, JS_NewObjectClass(context, classId)};
    if (object.isException())
        return object;

    auto* taskEntry = evaluation.findTaskEntry(taskId);
    if (!taskEntry)
        return jsThrowError("Task no longer exists");

    // Read-only property
    if (!object.defineProperty("promise", taskEntry->promise.get(), JS_PROP_ENUMERABLE))
        return jsThrowError("Failed to install task promise");

    if (!(object.setFunction("onProgress", &QuickJsTask::onProgress, 1)
          && object.setFunction("cancel", &QuickJsTask::cancel, 0)
          && object.setFunction("then", &QuickJsTask::then, 2)
        ))
    {
        return jsThrowError("Failed to initialize MayoTask");
    }

    auto handle = std::make_unique<TaskHandle>(&evaluation, taskId);
    object.setOpaquePtr(handle.get());
    handle.release();

    return object;
}

void QuickJsTask::finalizer(JSRuntime* runtime, JSValue value)
{
    auto classId = QuickJsScriptEngine::findClassId(runtime, className);
    if (classId != JS_INVALID_CLASS_ID)
        delete static_cast<TaskHandle*>(JS_GetOpaque(value, classId));
}

JSValue QuickJsTask::onProgress(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    auto handle = TaskHandle::getOpaquePtrOrThrow(context, thisValue);
    if (!handle)
        return JS_EXCEPTION;

    if (argc < 1)
        return JS_ThrowTypeError(context, "Wrong argument acount provided to callback function");

    auto taskEntry = handle->evaluation->findTaskEntry(handle->taskId);
    if (!taskEntry)
        return JS_ThrowInternalError(context, "Task no longer exists");

    QuickJsValue funcValue = QuickJsValue::dup(context, argv[0]);
    if (!funcValue.isFunction())
        return JS_ThrowTypeError(context, "Expected a callback function");

    taskEntry->progressCallbacks.push_back(std::move(funcValue));
    return JS_DupValue(context, thisValue);
}

JSValue QuickJsTask::cancel(JSContext* context, JSValueConst thisValue, int, JSValueConst*)
{
    auto handle = TaskHandle::getOpaquePtrOrThrow(context, thisValue);
    if (!handle)
        return JS_EXCEPTION;

    auto taskEntry = handle->evaluation->findTaskEntry(handle->taskId);
    if (taskEntry)
        handle->evaluation->m_taskMgr.requestAbort(handle->taskId);

    return JS_UNDEFINED;
}

JSValue QuickJsTask::then(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (!TaskHandle::getOpaquePtrOrThrow(context, thisValue))
        return JS_EXCEPTION;

    QuickJsValue promise{ context, JS_GetPropertyStr(context, thisValue, "promise") };
    if (promise.isException())
        return JS_EXCEPTION;

    QuickJsValue thenFunction = promise.getProperty("then");
    if (thenFunction.isException())
        return JS_EXCEPTION;

    return thenFunction.call(promise.get(), argc, argv).release();
}

} // namespace Mayo
