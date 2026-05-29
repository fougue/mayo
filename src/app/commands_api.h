/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/document.h"
#include "../base/text_id.h"

#include <QtCore/QObject>
#include <QAction> // WARNING Qt5 <QtWidgets/...> / Qt6 <QtGui/...>

#include <memory>
#include <unordered_map>
#include <type_traits>

class QWidget;

namespace Mayo {

class Application;
class GuiApplication;
class GuiDocument;
class V3dViewController;
class TaskManager;

class Command;

//
// Provides interface to access/interact with application state and services
//
// IAppContext acts as a central access point for application-wide resources such as the GUI
// application, task manager, document management, and main UI widgets.
// It decouples high-level components(Command objects) from concrete UI implementations(MainWindow)
//
// The interface intentionally exposes both high-level concepts(documents) and UI-related
// elements(widgets), acting as a pragmatic bridge layer
// State changes are not automatically propagated; callers must explicitly invoke
// updateControlsEnabledStatus() when needed
//
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

    virtual Document::Identifier currentDocument() const = 0;
    virtual void setCurrentDocument(Document::Identifier docId) = 0;

    virtual int findDocumentIndex(Document::Identifier docId) const = 0;
    virtual Document::Identifier findDocumentFromIndex(int index) const = 0;

    // Must be called whenever application state changes in a way that may affect
    // command enabled/disabled status(eg current document, selection, mode, etc.)
    virtual void updateControlsEnabledStatus() = 0;

signals:
    void currentDocumentChanged(Mayo::Document::Identifier docId);
};

//
// Represents a single action in the application
//
// A Command object encapsulates a unit of user-triggered behavior(eg menu action, toolbar button,
// shortcut). It provides both the execution logic and the optional UI binding through a QAction.
// Commands are typically created and owned by CommandContainer and operate in the context of an
// IAppContext, which gives access to application state and services.
//
class Command : public QObject {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::Command)
public:
    explicit Command(IAppContext* context);
    ~Command() override = default;

    // Executes the command logic(implemented by derived classes)
    // Typically triggered by the associated QAction
    virtual void execute() = 0;

    // Returns the application context associated with this command
    IAppContext* context() const { return m_context; }

    // Returns the associated QAction, if any
    // May return nullptr if createAction() has not been called
    QAction* action() const { return m_action; }

    // Returns whether the command is currently enabled.
    // Can be overridden to implement dynamic enable/disable logic based on application state
    // The returned value is applied to the QAction when updateEnabled() is called
    virtual bool getEnabledStatus() const { return true; }

    // Updates the enabled state of the associated QAction
    // If a QAction exists, its enabled state is set according to getEnabledStatus()
    // Does nothing if no action has been created
    void updateEnabled();

protected:
    Application* app() const;
    GuiApplication* guiApp() const { return m_context ? m_context->guiApp() : nullptr; }
    TaskManager* taskMgr() const { return m_context ? m_context->taskMgr() : nullptr; }
    QWidget* widgetMain() const { return m_context ? m_context->widgetMain() : nullptr; }
    Document::Identifier currentDocument() const { return m_context ? m_context->currentDocument() : -1; }
    GuiDocument* currentGuiDocument() const;
    int currentDocumentIndex() const;

    void setCurrentDocument(const DocumentPtr& doc);

    // Creates(if needed) and returns the QAction associated with this command
    // The action is owned by the Command by QObject parent-child mechanism(parent=this)
    // The action is connected to execute()
    // Subsequent calls return the same QAction instance
    QAction* createAction();

private:
    IAppContext* m_context = nullptr;
    QAction* m_action = nullptr;
};

//
// Provides an associative container for Command objects
//
// Each command is identified by a unique key("command name") and can be retrieved later using that
// identifier. Commands are typically registered once during application initialization and then
// queried or iterated over as needed.
//
// The container owns the Command instances it creates and is responsible for their lifetime. All
// commands are destroyed when clear() is called or when the container itself is destroyed.
//
class CommandContainer {
public:
    CommandContainer() = default;
    explicit CommandContainer(IAppContext* appContext);
    ~CommandContainer();

    CommandContainer(const CommandContainer&) = delete;
    CommandContainer& operator=(const CommandContainer&) = delete;

    IAppContext* appContext() const { return m_appContext; }
    void setAppContext(IAppContext* appContext);

    // Iterates over all commands stored in the container and applies the given function to each entry.
    // Example:
    //     container.foreachCommand([](std::string_view name, Command* cmd) { ... });
    // Notes:
    //   - The iteration order is unspecified(depends on std::unordered_map)
    //   - The container must not be modified during iteration(no calls to addNamedCommand(), clear(), ...)
    //   - The function `fn` must not delete the Command objects
    template<typename Function> void foreachCommand(Function fn) const;

    // Returns the Command object mapped to 'name'
    // That object was previously created and associated with a call to addCommand()/addNamedCommand()
    // Might return null in case no command is mapped to 'name'
    Command* findCommand(std::string_view name) const;

    // Helper function to retrieve the action provided by the Command object mapped to 'name'
    // Might return null in case no command is mapped to 'name'
    QAction* findCommandAction(std::string_view name) const;

    // Construct and register new Command of type `CmdType`
    // The command is created using the provided arguments `args...` and associated with its
    // implicit name, defined by `CmdType::Name`. That name must refer to static storage, eg string
    // literal or static constexpr string
    // The newly created command is owned by the container and can later be retrieved using findCommand()
    template<typename CmdType, typename... Args> CmdType* addNamedCommand(Args&&... args);

    // Deletes all commands owned by the container and clears the internal mapping
    // Notes:
    //   - All Command pointers previously returned by the container become invalid
    //   - Associated QAction objects(if any) owned by the commands are also destroyed
    //   - The container remains valid and can be reused to register new commands
    //   - This function must not be called while iterating over the container
    void clear();

private:
    // Helper to check CmdType::Name exists and is convertible to std::string_view
    template<typename, typename = std::void_t<>>
    struct has_valid_name : std::false_type {};

    // Helper to check CmdType::Name exists and is convertible to std::string_view
    template<typename T>
    struct has_valid_name<T, std::void_t<decltype(T::Name)>>
        : std::bool_constant<std::is_convertible_v<decltype(T::Name), std::string_view>> {}
    ;

    // Construct and add new Command object with arguments 'args'
    // The command is associated to identifier 'name' and can be retrieved later on with findCommand()
    // IMPORTANT `name` must refer to a static string(eg CmdType::Name)
    template<typename CmdType, typename... Args>
    CmdType* addCommand(std::string_view name, Args&&... args);

    void addCommand_impl(std::string_view name, std::unique_ptr<Command> cmd);

    IAppContext* m_appContext = nullptr;
    std::unordered_map<std::string_view, std::unique_ptr<Command>> m_mapCommand;
};



// --
// -- Implementation
// --

template<typename Function>
void CommandContainer::foreachCommand(Function fn) const
{
    for (const auto& [name, cmd] : m_mapCommand) {
        fn(name, cmd.get());
    }
}

template<typename CmdType, typename... Args>
CmdType* CommandContainer::addCommand(std::string_view name, Args&&... args)
{
    auto cmd = std::make_unique<CmdType>(m_appContext, std::forward<Args>(args)...);
    auto cmdPtr = cmd.get();
    this->addCommand_impl(name, std::move(cmd));
    return cmdPtr;
}

template<typename CmdType, typename... Args>
CmdType* CommandContainer::addNamedCommand(Args&&... args)
{
    static_assert(
        has_valid_name<CmdType>::value,
        "CmdType::Name must exist and be convertible to std::string_view"
    );
    return this->addCommand<CmdType>(CmdType::Name, std::forward<Args>(args)...);
}

} // namespace Mayo
