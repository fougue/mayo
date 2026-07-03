/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class CommandReportBug : public Command {
public:
    explicit CommandReportBug(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "report-bug";
};

class CommandSystemInformation : public Command {
public:
    explicit CommandSystemInformation(IAppContext* context);
    void execute() override;

    static QString data();
    static constexpr std::string_view Name = "system-info";
};

class CommandAbout : public Command {
public:
    explicit CommandAbout(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "about";
};

} // namespace Mayo
