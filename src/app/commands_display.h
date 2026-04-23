/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"

#include <string_view>

class QMenu;

namespace Mayo {

class BaseCommandDisplay : public Command {
public:
    BaseCommandDisplay(IAppContext* context);
    bool getEnabledStatus() const override;
};

class CommandChangeProjection : public BaseCommandDisplay {
public:
    CommandChangeProjection(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "change-projection";

private:
    void onCurrentDocumentChanged();
    QAction* m_actionOrtho = nullptr;
    QAction* m_actionPersp = nullptr;
};

class CommandChangeDisplayMode : public BaseCommandDisplay {
public:
    CommandChangeDisplayMode(IAppContext* context);
    CommandChangeDisplayMode(IAppContext* context, QMenu* containerMenu);
    void execute() override;

    static constexpr std::string_view Name = "change-display-mode";

private:
    void recreateMenuDisplayMode();
};

class CommandToggleOriginTrihedron : public BaseCommandDisplay {
public:
    CommandToggleOriginTrihedron(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "toggle-origin-trihedron";

private:
    void onCurrentDocumentChanged();
};

class CommandTogglePerformanceStats : public BaseCommandDisplay {
public:
    CommandTogglePerformanceStats(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "toggle-performance-stats";

private:
    void onCurrentDocumentChanged();
};

class CommandZoomInCurrentDocument : public BaseCommandDisplay {
public:
    CommandZoomInCurrentDocument(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "current-doc-zoom-in";
};

class CommandZoomOutCurrentDocument : public BaseCommandDisplay {
public:
    CommandZoomOutCurrentDocument(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "current-doc-zoom-out";
};

class CommandTurnViewCounterClockWise : public BaseCommandDisplay {
public:
    CommandTurnViewCounterClockWise(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "current-doc-turn-view-ccw";
};

class CommandTurnViewClockWise : public BaseCommandDisplay {
public:
    CommandTurnViewClockWise(IAppContext* context);
    void execute() override;

    static constexpr std::string_view Name = "current-doc-turn-view-cw";
};

} // namespace Mayo
