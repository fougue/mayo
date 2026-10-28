/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"

namespace Mayo {

class QuickJsScriptEvaluation;
class ScriptHostEnvironment;

QuickJsValue quickjs_installMayoApi(
    QuickJsScriptEvaluation& evaluation, const ScriptHostEnvironment& hostEnv
);

} // namespace Mayo
