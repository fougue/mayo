/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "iscript_engine.h"

namespace Mayo {

class NullScriptEngine : public IScriptEngine {
public:
    void startEvaluate() override;
    void stopEvaluate() override;
    bool isEvaluateRunning() const override;
    bool waitForEvaluateEnd(int msecs = -1) override;
};

} // namespace Mayo
