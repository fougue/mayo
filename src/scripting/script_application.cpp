/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "script_application.h"

namespace Mayo {

int ScriptApplication::documentCount() const
{
    return m_app ? m_app->documentCount() : 0;
}

ScriptDocument ScriptApplication::newDocument()
{
    return ScriptDocument{};
}

} // namespace Mayo
