/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_api.h"

#include "../base/application.h"
#include "../gui/gui_application.h"

#include <QtWidgets/QWidget>
#include <fmt/format.h>
#include <stdexcept>

namespace Mayo {

IAppContext::IAppContext(QObject* parent)
    : QObject(parent)
{
}

Command::Command(IAppContext* context)
    : QObject(context ? context->widgetMain() : nullptr),
      m_context(context)
{
}

Application* Command::app() const
{
    return m_context ? m_context->guiApp()->application().get() : nullptr;
}

GuiDocument* Command::currentGuiDocument() const
{
    DocumentPtr doc = this->app()->findDocumentByIdentifier(this->currentDocument());
    return this->guiApp()->findGuiDocument(doc);
}

int Command::currentDocumentIndex() const
{
    return m_context ? m_context->findDocumentIndex(this->currentDocument()) : -1;
}

void Command::setCurrentDocument(const DocumentPtr& doc)
{
    m_context->setCurrentDocument(doc->identifier());
}

void Command::setAction(QAction* action)
{
    m_action = action;
    QObject::connect(action, &QAction::triggered, this, &Command::execute);
}

CommandContainer::CommandContainer(IAppContext* appContext)
    : m_appContext(appContext)
{
}

void CommandContainer::setAppContext(IAppContext* appContext)
{
    assert(!m_appContext && m_mapCommand.empty());
    m_appContext = appContext;
}

Command* CommandContainer::findCommand(std::string_view name) const
{
    auto it = m_mapCommand.find(name);
    return it != m_mapCommand.cend() ? it->second : nullptr;
}

QAction* CommandContainer::findCommandAction(std::string_view name) const
{
    auto cmd = this->findCommand(name);
    return cmd ? cmd->action() : nullptr;
}

void CommandContainer::clear()
{
    for (auto [name, cmd] : m_mapCommand) {
        delete cmd;
    }

    m_mapCommand.clear();
}

void CommandContainer::addCommand_impl(std::string_view name, Command* cmd)
{
    assert(m_appContext != nullptr);
    auto [it, ok] = m_mapCommand.insert({ name, cmd });
    if (!ok)
        throw std::invalid_argument(fmt::format("Command name {} already exists", name));
}

} // namespace Mayo
