/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "commands_api.h"
#include "../base/filepath.h"
#include "../base/messenger.h"
#include "../base/task_manager.h"
#include "../base/text_id.h"

#include <QtWidgets/QMainWindow>

#include <functional>
#include <unordered_map>
#include <vector>

namespace Mayo {

class AppUiState;
class GuiApplication;
class GuiDocument;
class IWidgetMainPage;
class WidgetMainControl;
class WidgetMainHome;

// Provides the root widget of the application GUI
// It creates and owns the various available commands(actions)
class MainWindow : public QMainWindow {
    Q_OBJECT
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::MainWindow)
public:
    MainWindow(GuiApplication* guiApp, QWidget* parent = nullptr);
    ~MainWindow();

    // Opens the documents referenced by `listFilePath`
    // Each file path is forwarded to the document asynchronous loading/import mechanism
    // Unsupported, missing, or invalid files may generate warning/error messages through the
    // application's messaging system
    void openDocumentsFromList(gsl::span<const FilePath> listFilePath);

    // Restores the persistent UI state of the main window
    // This automatically calls IWidgetMainPage::restoreUiState() on all widget pages
    void restoreUiState(const AppUiState& state);

    // Saves the current UI state of the main window into `state`
    // This automatically calls IWidgetMainPage::saveUiState() on all widget pages
    void saveUiState(AppUiState& state);

    // Displays message to the user
    // Depending on the message type and configuration, the message may appear as popup, dialog, ...
    void showMessage(const Messenger::Message& msg);

    // Registers callback invoked when the main window is closing with QWidget::closeEvent()
    void addOnCloseCallback(std::function<void()> fn);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void addPage(IAppContext::Page page, IWidgetMainPage* pageWidget);

    void createCommands();
    void createMenus();
    template<typename CmdType, typename... Args> void addCommand(Args&&... args) {
        m_cmdContainer.addNamedCommand<CmdType>(std::forward<Args>(args)...);
    }

    void onOperationFinished(bool ok, const QString& msg);
    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onGuiDocumentErased(GuiDocument* guiDoc);

    void updateControlsActivation();
    void updateCurrentPage();

    IWidgetMainPage* widgetMainPage(IAppContext::Page page) const;
    WidgetMainHome* widgetPageHome() const;
    WidgetMainControl* widgetPageDocuments() const;

    friend class AppContext;

    IAppContext* m_appContext = nullptr;
    GuiApplication* m_guiApp = nullptr;
    CommandContainer m_cmdContainer;
    TaskManager m_taskMgr;
    class Ui_MainWindow* m_ui = nullptr;
    std::unordered_map<IAppContext::Page, IWidgetMainPage*> m_mapWidgetPage;
    std::vector<std::function<void()>> m_onCloseCallbacks;
};

} // namespace Mayo
