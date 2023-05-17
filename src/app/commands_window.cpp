/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_window.h"

#include "../base/application.h"
#include "theme.h"

#include <QtGui/QShowEvent>
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
    action->setToolTip(Command::tr("Show/Hide Left Sidebar"));
    action->setShortcut(Qt::ALT + Qt::Key_0);
    action->setCheckable(true);
    action->setChecked(context->widgetLeftSidebar()->isVisible());
    this->setAction(action);
    this->updateAction();
    context->widgetLeftSidebar()->installEventFilter(this);
}

void CommandLeftSidebarWidgetToggle::execute()
{
    QWidget* widget = this->context()->widgetLeftSidebar();
    widget->setVisible(!widget->isVisible());
}

bool CommandLeftSidebarWidgetToggle::getEnabledStatus() const
{
    return this->context()->modeWidgetMain() != IAppContext::ModeWidgetMain::Home;
}

bool CommandLeftSidebarWidgetToggle::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this->context()->widgetLeftSidebar()) {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide) {
            this->updateAction();
            return true;
        }
        else {
            return false;
        }
    }

    return Command::eventFilter(watched, event);
}

void CommandLeftSidebarWidgetToggle::updateAction()
{
    if (this->context()->widgetLeftSidebar()->isVisible()) {
        this->action()->setText(Command::tr("Hide Left Sidebar"));
        this->action()->setIcon(mayoTheme()->icon(Theme::Icon::BackSquare));
    }
    else {
        this->action()->setText(Command::tr("Show Left Sidebar"));
        this->action()->setIcon(mayoTheme()->icon(Theme::Icon::LeftSidebar));
    }

    this->action()->setToolTip(this->action()->text());
}

CommandSwitchMainWidgetMode::CommandSwitchMainWidgetMode(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setToolTip(Command::tr("Go To Home Page"));
    action->setShortcut(Qt::CTRL + Qt::Key_0);
    this->setAction(action);
    this->updateAction();
    context->widgetMainByMode(IAppContext::ModeWidgetMain::Home)->installEventFilter(this);
    context->widgetMainByMode(IAppContext::ModeWidgetMain::Documents)->installEventFilter(this);
}

void CommandSwitchMainWidgetMode::execute()
{
    auto newMode = IAppContext::ModeWidgetMain::Unknown;
    switch (this->context()->modeWidgetMain()) {
    case IAppContext::ModeWidgetMain::Home:
        newMode = IAppContext::ModeWidgetMain::Documents;
        break;
    case IAppContext::ModeWidgetMain::Documents:
        newMode = IAppContext::ModeWidgetMain::Home;
        break;
    }

    this->context()->setModeWidgetMain(newMode);
}

bool CommandSwitchMainWidgetMode::getEnabledStatus() const
{
    return this->app()->documentCount() != 0;
}

bool CommandSwitchMainWidgetMode::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == this->context()->widgetMainByMode(IAppContext::ModeWidgetMain::Home)
            || watched == this->context()->widgetMainByMode(IAppContext::ModeWidgetMain::Documents))
    {
        if (event->type() == QEvent::Show) {
            this->updateAction();
            return true;
        }
        else {
            return false;
        }
    }

    return Command::eventFilter(watched, event);
}

void CommandSwitchMainWidgetMode::updateAction()
{
    switch (this->context()->modeWidgetMain()) {
    case IAppContext::ModeWidgetMain::Home:
        this->action()->setText(Command::tr("Go To Documents"));
        break;
    case IAppContext::ModeWidgetMain::Documents:
        this->action()->setText(Command::tr("Go To Home Page"));
        break;
    }
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
