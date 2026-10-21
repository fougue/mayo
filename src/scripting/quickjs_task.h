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

class QuickJsTask {
public:
    static QuickJsValue create(QuickJsScriptEvaluation& evaluation, TaskId taskId);

    static void finalizer(JSRuntime* runtime, JSValue value);

    static constexpr const char* className = "MayoTask";

private:
    // Bindings
    static JSValue onProgress(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );
    static JSValue cancel(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );
    static JSValue then(
        JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv
    );
};

} // namespace Mayo
