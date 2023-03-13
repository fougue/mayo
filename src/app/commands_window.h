/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "commands_api.h"

namespace Mayo {

class CommandMainWidgetToggleFullscreen : public Command {
public:
    CommandMainWidgetToggleFullscreen(IAppContext* context);
    void execute() override;

private:
    Qt::WindowStates m_previousWindowState = Qt::WindowNoState;
};

class CommandLeftSidebarWidgetToggle : public Command {
public:
    CommandLeftSidebarWidgetToggle(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateAction();
};

class CommandPreviousDocument : public Command {
public:
    CommandPreviousDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;
};

class CommandNextDocument : public Command {
public:
    CommandNextDocument(IAppContext* context);
    void execute() override;
    bool getEnabledStatus() const override;
};

} // namespace Mayo
