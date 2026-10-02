/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/application.h"

namespace Mayo {

class ScriptDocument {
};

class ScriptApplication {
public:
    //std::string semanticVersion() const;
    int documentCount() const;
    ScriptDocument newDocument();

private:
    ApplicationPtr m_app;
};

} // namespace Mayo