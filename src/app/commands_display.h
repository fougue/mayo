/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "commands_api.h"

#include <string_view>

class QMenu;

namespace Mayo {

class CommandChangeProjection : public Command {
public:
    CommandChangeProjection(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "change-projection";

private:
    void onCurrentDocumentChanged();
    QAction* m_actionOrtho = nullptr;
    QAction* m_actionPersp = nullptr;
};

class CommandChangeDisplayMode : public Command {
public:
    CommandChangeDisplayMode(IAppContext* context);
    CommandChangeDisplayMode(IAppContext* context, QMenu* containerMenu);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "change-display-mode";

private:
    void recreateMenuDisplayMode();
};

class CommandToggleOriginTrihedron : public Command {
public:
    CommandToggleOriginTrihedron(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "toggle-origin-trihedron";

private:
    void onCurrentDocumentChanged();
};

class CommandTogglePerformanceStats : public Command {
public:
    CommandTogglePerformanceStats(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "toggle-performance-stats";

private:
    void onCurrentDocumentChanged();
};

class CommandZoomInCurrentDocument : public Command {
public:
    CommandZoomInCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "current-doc-zoom-in";
};

class CommandZoomOutCurrentDocument : public Command {
public:
    CommandZoomOutCurrentDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "current-doc-zoom-out";
};

class CommandTurnViewCounterClockWise : public Command {
public:
    CommandTurnViewCounterClockWise(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "current-doc-turn-view-ccw";
};

class CommandTurnViewClockWise : public Command {
public:
    CommandTurnViewClockWise(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    static constexpr std::string_view Name = "current-doc-turn-view-cw";
};

} // namespace Mayo
