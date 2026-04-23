/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class CommandSaveViewImage : public Command {
public:
    CommandSaveViewImage(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "save-view-image";
};

class CommandInspectXde : public Command {
public:
    CommandInspectXde(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "inspect-xde";
};

class CommandEditOptions : public Command {
public:
    CommandEditOptions(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "edit-options";
};

} // namespace Mayo
