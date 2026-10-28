/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"

#include "../base/task_common.h"

#include <quickjs.h>

namespace Mayo {

class QuickJsScriptEvaluation;

// Creates the QuickJS class used to expose asynchronous TaskManager tasks to JavaScript
// The resulting object is "thenable" so it can be used with `await` and also provides progress
// notifications and cancellation. Its opaque handle refers to a task owned by a
// QuickJsScriptEvaluation so the object must only be used while that evaluation is alive
class QuickJsTask {
public:
    // Creates a JavaScript task object for `taskId` in `evaluation`
    // Returns a JavaScript exception if the class is not registered, the task is unavailable, or
    // the object cannot be initialized
    static QuickJsValue create(QuickJsScriptEvaluation& evaluation, TaskId taskId);

    // Releases the opaque task handle attached to a JavaScript task object
    static void finalizer(JSRuntime* runtime, JSValue value);

    // QuickJS class name used when registering and looking up this class
    static constexpr const char* className = "MayoTask";

private:
    // Bindings

    // Registers a callback receiving progress objects with `value` and `message` properties
    static JSValue onProgress(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );

    // Requests cancellation of the associated task. The task may take time to observe the request
    static JSValue cancel(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );

    // Forwards `then()` calls to the task's internal Promise, making the object awaitable
    static JSValue then(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );
};

} // namespace Mayo
