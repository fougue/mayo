/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class CommandReportBug : public Command {
public:
    CommandReportBug(IAppContext* context);
    void execute() override;
};

class CommandAbout : public Command {
public:
    CommandAbout(IAppContext* context);
    void execute() override;
};

} // namespace Mayo
