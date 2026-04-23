/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class CommandMainWidgetToggleFullscreen : public Command {
public:
    CommandMainWidgetToggleFullscreen(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "toggle-fullscreen";

private:
    Qt::WindowStates m_previousWindowState = Qt::WindowNoState;
};

// Provides command to toggle visibility of the left-side bar widget belonging to
// the "Documents" main page
class CommandLeftSidebarWidgetToggle : public Command {
public:
    CommandLeftSidebarWidgetToggle(IAppContext* context);

    void execute() override;
    bool getEnabledStatus() const override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    static constexpr std::string_view Name = "toggle-left-sidebar";

private:
    void updateAction();
};

class CommandSwitchMainWidgetMode : public Command {
public:
    CommandSwitchMainWidgetMode(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    static constexpr std::string_view Name = "switch-main-widget-mode";

private:
    void updateAction();
};

class CommandPreviousDocument : public Command {
public:
    CommandPreviousDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "previous-doc";
};

class CommandNextDocument : public Command {
public:
    CommandNextDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "next-doc";
};

} // namespace Mayo
