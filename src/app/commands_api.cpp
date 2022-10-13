/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_api.h"

#include "../base/application.h"
#include "../gui/gui_application.h"

#include <QtWidgets/QWidget>

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
    return m_context->guiApp()->application().get();
}

GuiDocument* Command::currentGuiDocument() const
{
    DocumentPtr doc = this->app()->findDocumentByIdentifier(this->currentDocument());
    return this->guiApp()->findGuiDocument(doc);
}

int Command::currentDocumentIndex() const
{
    return this->context()->findDocumentIndex(this->currentDocument());
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

} // namespace Mayo
