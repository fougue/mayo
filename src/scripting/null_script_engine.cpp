/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "null_script_engine.h"

namespace Mayo {

void NullScriptEngine::startEvaluate()
{
}

void NullScriptEngine::stopEvaluate()
{
}

bool NullScriptEngine::isEvaluateRunning() const
{
    return false;
}

bool NullScriptEngine::waitForEvaluateEnd(int /*msecs*/)
{
    return false;
}

} // namespace Mayo

