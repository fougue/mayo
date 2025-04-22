/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../base/filepath.h"
#include "../base/text_id.h"

#include <QtCore/QObject>
#include <QAction> // WARNING Qt5 <QtWidgets/...> / Qt6 <QtGui/...>
#include <unordered_map>
class QWidget;

namespace Mayo {

class Application;
class GuiApplication;
class GuiDocument;
class V3dViewController;
class TaskManager;

class Command;

// Provides interface to access/interact with application
class IAppContext : public QObject {
    Q_OBJECT
public:
    enum class Page { Unknown, Home, Documents };

    IAppContext(QObject* parent = nullptr);

    virtual GuiApplication* guiApp() const = 0;
    virtual TaskManager* taskMgr() const = 0;
    virtual V3dViewController* v3dViewController(const GuiDocument* guiDoc) const = 0;

    virtual QWidget* widgetMain() const = 0;
    virtual QWidget* widgetPage(Page page) const = 0;
    virtual Page currentPage() const = 0;
    virtual void setCurrentPage(Page page) = 0;
    virtual QWidget* pageDocuments_widgetLeftSideBar() const = 0;

    virtual Document::Identifier currentDocument() const = 0;
    virtual void setCurrentDocument(Document::Identifier docId) = 0;

    virtual int findDocumentIndex(Document::Identifier docId) const = 0;
    virtual Document::Identifier findDocumentFromIndex(int index) const = 0;

    virtual void updateControlsEnabledStatus() = 0;

signals:
    void currentDocumentChanged(Mayo::Document::Identifier docId);
};

// Represents a single action in the application
class Command : public QObject {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Command)
public:
    Command(IAppContext* context);
    virtual ~Command() = default;

    virtual void execute() = 0;

    IAppContext* context() const { return m_context; }

    QAction* action() const { return m_action; }
    virtual bool getEnabledStatus() const { return true; }

protected:
    Application* app() const;
    GuiApplication* guiApp() const { return m_context ? m_context->guiApp() : nullptr; }
    TaskManager* taskMgr() const { return m_context ? m_context->taskMgr() : nullptr; }
    QWidget* widgetMain() const { return m_context ? m_context->widgetMain() : nullptr; }
    Document::Identifier currentDocument() const { return m_context ? m_context->currentDocument() : -1; }
    GuiDocument* currentGuiDocument() const;
    int currentDocumentIndex() const;

    void setCurrentDocument(const DocumentPtr& doc);
    void setAction(QAction* action);

private:
    IAppContext* m_context = nullptr;
    QAction* m_action = nullptr;
};

// Provides an associative container dedicated to Command objects
// Each command in the container is mapped to a unique identifier(ie the "command name")
class CommandContainer {
public:
    CommandContainer() = default;
    CommandContainer(IAppContext* appContext);

    IAppContext* appContext() const { return m_appContext; }
    void setAppContext(IAppContext* appContext);

    template<typename Function> void foreachCommand(Function fn);

    // Returns the Command object mapped to 'name'
    // That object was previously created and associated with a call to addCommand()/addNamedCommand()
    // Might return null in case no command is mapped to 'name'
    Command* findCommand(std::string_view name) const;

    // Helper function to retrieve the action provided by the Command object mapped to 'name'
    // Might return null in case no command is mapped to 'name'
    QAction* findCommandAction(std::string_view name) const;

    // Construct and add new Command object with arguments 'args'
    // The command is associated to identigfier 'name' and can be retrieved later on with findCommand()
    template<typename CmdType, typename... Args> CmdType* addCommand(std::string_view name, Args... p);

    // Same behavior as addCommand() function
    // The command name is implicit and found by assuming the presence of CmdType::Name class member
    template<typename CmdType, typename... Args> CmdType* addNamedCommand(Args... p);

    void clear();

private:
    void addCommand_impl(std::string_view name, Command* cmd);

    IAppContext* m_appContext = nullptr;
    std::unordered_map<std::string_view, Command*> m_mapCommand;
};



// --
// -- Implementation
// --

template<typename Function>
void CommandContainer::foreachCommand(Function fn)
{
    for (auto [name, cmd] : m_mapCommand) {
        fn(name, cmd);
    }
}

template<typename CmdType, typename... Args>
CmdType* CommandContainer::addCommand(std::string_view name, Args... p)
{
    auto cmd = new CmdType(m_appContext, p...);
    this->addCommand_impl(name, cmd);
    return cmd;
}

template<typename CmdType, typename... Args>
CmdType* CommandContainer::addNamedCommand(Args... p)
{
    auto cmd = new CmdType(m_appContext, p...);
    this->addCommand_impl(CmdType::Name, cmd);
    return cmd;
}

} // namespace Mayo
