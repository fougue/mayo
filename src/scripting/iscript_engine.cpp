/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "iscript_engine.h"

namespace Mayo {

void IScriptEngine::setScriptFilePath(const FilePath& filePath)
{
    m_scriptFilePath = filepathCanonical(filePath).make_preferred();
}

void IScriptEngine::startOrStopEvaluate()
{
    if (this->isEvaluateRunning())
        this->stopEvaluate();
    else
        this->startEvaluate();
}

} // namespace Mayo
