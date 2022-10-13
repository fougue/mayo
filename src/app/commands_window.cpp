/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_window.h"

#include "../base/application.h"
#include "theme.h"

#include <QtWidgets/QWidget>

namespace Mayo {

CommandMainWidgetToggleFullscreen::CommandMainWidgetToggleFullscreen(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Fullscreen"));
    action->setToolTip(Command::tr("Switch Fullscreen/Normal"));
    action->setShortcut(Qt::Key_F11);
    action->setCheckable(true);
    action->setChecked(context->widgetMain()->isFullScreen());
    this->setAction(action);
}

void CommandMainWidgetToggleFullscreen::execute()
{
    auto widget = this->widgetMain();
    if (widget->isFullScreen()) {
        if (m_previousWindowState.testFlag(Qt::WindowMaximized))
            widget->showMaximized();
        else
            widget->showNormal();
    }
    else {
        m_previousWindowState = widget->windowState();
        widget->showFullScreen();
    }
}

CommandLeftSidebarWidgetToggle::CommandLeftSidebarWidgetToggle(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Show Left Sidebar"));
    action->setToolTip(Command::tr("Show/Hide Left Sidebar"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::LeftSidebar));
    action->setShortcut(Qt::ALT + Qt::Key_0);
    action->setCheckable(true);
    action->setChecked(context->widgetLeftSidebar()->isVisible());
    this->setAction(action);
}

void CommandLeftSidebarWidgetToggle::execute()
{
    const bool isVisible = this->context()->widgetLeftSidebar()->isVisible();
    this->context()->widgetLeftSidebar()->setVisible(!isVisible);
}

bool CommandLeftSidebarWidgetToggle::getEnabledStatus() const
{
    return this->context()->modeWidgetMain() != IAppContext::ModeWidgetMain::Home;
}

CommandPreviousDocument::CommandPreviousDocument(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Previous Document"));
    action->setToolTip(Command::tr("Previous Document"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Back));
    action->setShortcut(Qt::ALT + Qt::Key_Left);
    this->setAction(action);
}

void CommandPreviousDocument::execute()
{
    const int prevDocIndex = this->currentDocumentIndex() - 1;
    this->context()->setCurrentDocument(this->context()->findDocumentFromIndex(prevDocIndex));
}

bool CommandPreviousDocument::getEnabledStatus() const
{
    return this->app()->documentCount() != 0 && this->currentDocumentIndex() > 0;
}

CommandNextDocument::CommandNextDocument(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Next Document"));
    action->setToolTip(Command::tr("Next Document"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Next));
    action->setShortcut(Qt::ALT + Qt::Key_Right);
    this->setAction(action);
}

void CommandNextDocument::execute()
{
    const int nextDocIndex = this->currentDocumentIndex() + 1;
    this->context()->setCurrentDocument(this->context()->findDocumentFromIndex(nextDocIndex));
}

bool CommandNextDocument::getEnabledStatus() const
{
    const int appDocumentCount = this->app()->documentCount();
    return appDocumentCount != 0 && this->currentDocumentIndex() < appDocumentCount - 1;
}

} // namespace Mayo
