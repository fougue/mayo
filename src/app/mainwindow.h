/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "commands_api.h"
#include "../base/filepath.h"
#include "../base/messenger.h"
#include "../base/task_manager.h"
#include "../base/text_id.h"
#include <QtWidgets/QMainWindow>
#include <unordered_map>

namespace Mayo {

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

    void openDocumentsFromList(Span<const FilePath> listFilePath);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void addPage(IAppContext::Page page, IWidgetMainPage* pageWidget);

    void createCommands();
    void createMenus();
    template<typename CmdType, typename... Args> void addCommand(Args... p) {
        m_cmdContainer.addNamedCommand<CmdType>(std::forward<Args>(p)...);
    }

    void onOperationFinished(bool ok, const QString& msg);
    void onGuiDocumentAdded(GuiDocument* guiDoc);
    void onGuiDocumentErased(GuiDocument* guiDoc);

    void onMessage(MessageType msgType, const QString& text);

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
};

} // namespace Mayo
