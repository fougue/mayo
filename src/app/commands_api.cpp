/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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

void Command::updateEnabled()
{
    if (m_action)
        m_action->setEnabled(this->getEnabledStatus());
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

QAction* Command::createAction()
{
    if (!m_action) {
        m_action = new QAction(this);
        QObject::connect(m_action, &QAction::triggered, this, &Command::execute);
    }

    return m_action;
}

CommandContainer::CommandContainer(IAppContext* appContext)
    : m_appContext(appContext)
{
}

CommandContainer::~CommandContainer()
{
    this->clear();
}

void CommandContainer::setAppContext(IAppContext* appContext)
{
    assert(!m_appContext && m_mapCommand.empty());
    m_appContext = appContext;
}

Command* CommandContainer::findCommand(std::string_view name) const
{
    auto it = m_mapCommand.find(name);
    return it != m_mapCommand.cend() ? it->second.get() : nullptr;
}

QAction* CommandContainer::findCommandAction(std::string_view name) const
{
    auto cmd = this->findCommand(name);
    return cmd ? cmd->action() : nullptr;
}

void CommandContainer::clear()
{
    m_mapCommand.clear();
}

void CommandContainer::addCommand_impl(std::string_view name, std::unique_ptr<Command> cmd)
{
    assert(m_appContext != nullptr);
    auto [it, ok] = m_mapCommand.emplace(name, std::move(cmd));
    if (!ok)
        throw std::invalid_argument(fmt::format("Command name {} already exists", name));
}

} // namespace Mayo
