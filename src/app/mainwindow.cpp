/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../base/application.h"
#include "../base/global.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "app_context.h"
#include "app_module.h"
#include "commands_file.h"
#include "commands_display.h"
#include "commands_tools.h"
#include "commands_window.h"
#include "commands_help.h"
#include "dialog_task_manager.h"
#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "theme.h"
#include "widget_main_control.h"
#include "widget_main_home.h"
#include "widget_message_indicator.h"

#ifdef Q_OS_WIN
#  include "windows/win_taskbar_global_progress.h"
#endif

#include <QtDebug>

namespace Mayo {

MainWindow::MainWindow(GuiApplication* guiApp, QWidget* parent)
    : QMainWindow(parent),
      m_guiApp(guiApp),
      m_ui(new Ui_MainWindow)
{
    m_ui->setupUi(this);
    this->addPage(IAppContext::Page::Home, new WidgetMainHome(this));
    this->addPage(IAppContext::Page::Documents, new WidgetMainControl(guiApp, this));

    // AppContext requires WidgetMainControl object, ensure it has been created beforehand
    m_appContext = new AppContext(this);
    m_cmdContainer.setAppContext(m_appContext);

    // Some commands requires WidgetMainControl UI page to exist, ensure it has been created beforehand
    this->createCommands();
    this->createMenus();

    // WidgetMainControl page depends on some Command objects, ensure they have been created beforehand
    for (auto [code, page] : m_mapWidgetPage)
        page->initialize(&m_cmdContainer);

    AppModule::get()->signalMessage.connectSlot(&MainWindow::onMessage, this);
    guiApp->signalGuiDocumentAdded.connectSlot(&MainWindow::onGuiDocumentAdded, this);
    guiApp->signalGuiDocumentErased.connectSlot(&MainWindow::onGuiDocumentErased, this);

    new DialogTaskManager(&m_taskMgr, this);

    this->updateControlsActivation();
}

MainWindow::~MainWindow()
{
    // Force deletion of Command objects as some of them are event filters of MainWindow widgets
    m_cmdContainer.clear();
    delete m_ui;
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
#if defined(Q_OS_WIN) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    constexpr Qt::FindChildOption findMode = Qt::FindDirectChildrenOnly;
    auto winProgress = this->findChild<WinTaskbarGlobalProgress*>(QString(), findMode);
    if (!winProgress)
        winProgress = new WinTaskbarGlobalProgress(&m_taskMgr, this);

    winProgress->setWindow(this->windowHandle());
#endif
}

void MainWindow::addPage(IAppContext::Page page, IWidgetMainPage* pageWidget)
{
    assert(m_mapWidgetPage.find(page) == m_mapWidgetPage.cend());
    assert(m_ui->stack_Main->indexOf(pageWidget) == -1);
    m_mapWidgetPage.insert({ page, pageWidget });
    m_ui->stack_Main->addWidget(pageWidget);
    QObject::connect(
                pageWidget, &IWidgetMainPage::updateGlobalControlsActivationRequired,
                this, &MainWindow::updateControlsActivation
    );
}

void MainWindow::createCommands()
{
    // "File" commands
    this->addCommand<CommandNewDocument>();
    this->addCommand<CommandOpenDocuments>();
    this->addCommand<CommandRecentFiles>(m_ui->menu_File);
    this->addCommand<CommandImportInCurrentDocument>();
    this->addCommand<CommandExportSelectedApplicationItems>();
    this->addCommand<CommandCloseCurrentDocument>();
    this->addCommand<CommandCloseAllDocuments>();
    this->addCommand<CommandCloseAllDocumentsExceptCurrent>();
    this->addCommand<CommandQuitApplication>();

    // "Display" commands
    this->addCommand<CommandChangeProjection>();
    this->addCommand<CommandChangeDisplayMode>(m_ui->menu_Display);
    this->addCommand<CommandToggleOriginTrihedron>();
    this->addCommand<CommandTogglePerformanceStats>();
    this->addCommand<CommandZoomInCurrentDocument>();
    this->addCommand<CommandZoomOutCurrentDocument>();
    this->addCommand<CommandTurnViewCounterClockWise>();
    this->addCommand<CommandTurnViewClockWise>();

    // "Tools" commands
    this->addCommand<CommandSaveViewImage>();
    this->addCommand<CommandInspectXde>();
    this->addCommand<CommandEditOptions>();
    this->addCommand<CommandExecScript>();
    this->addCommand<CommandExecRecentScript>(m_ui->menu_Tools);

    // "Window" commands
    this->addCommand<CommandLeftSidebarWidgetToggle>();
    this->addCommand<CommandMainWidgetToggleFullscreen>();
    this->addCommand<CommandSwitchMainWidgetMode>();
    this->addCommand<CommandPreviousDocument>();
    this->addCommand<CommandNextDocument>();

    // "Help" commands
    this->addCommand<CommandReportBug>();
    this->addCommand<CommandSystemInformation>();
    this->addCommand<CommandAbout>();
}

void MainWindow::createMenus()
{
    // Helper function to add in 'menu' the QAction associated to 'commandName'
    auto fnAddAction = [=](QMenu* menu, std::string_view commandName) {
        menu->addAction(m_cmdContainer.findCommandAction(commandName));
    };

    // TODO Create menu bar programmatically(not hard-code in .ui file)

    {   // File
        auto menu = m_ui->menu_File;
        fnAddAction(menu, CommandNewDocument::Name);
        fnAddAction(menu, CommandOpenDocuments::Name);
        fnAddAction(menu, CommandRecentFiles::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandImportInCurrentDocument::Name);
        fnAddAction(menu, CommandExportSelectedApplicationItems::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandCloseCurrentDocument::Name);
        fnAddAction(menu, CommandCloseAllDocumentsExceptCurrent::Name);
        fnAddAction(menu, CommandCloseAllDocuments::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandQuitApplication::Name);
    }

    {   // Display
        auto menu = m_ui->menu_Display;
        fnAddAction(menu, CommandChangeProjection::Name);
        fnAddAction(menu, CommandChangeDisplayMode::Name);
        fnAddAction(menu, CommandToggleOriginTrihedron::Name);
        fnAddAction(menu, CommandTogglePerformanceStats::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandZoomInCurrentDocument::Name);
        fnAddAction(menu, CommandZoomOutCurrentDocument::Name);
        fnAddAction(menu, CommandTurnViewCounterClockWise::Name);
        fnAddAction(menu, CommandTurnViewClockWise::Name);
    }

    {   // Tools
        auto menu = m_ui->menu_Tools;
        fnAddAction(menu, CommandSaveViewImage::Name);
        fnAddAction(menu, CommandInspectXde::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandExecScript::Name);
        fnAddAction(menu, CommandExecRecentScript::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandEditOptions::Name);
    }

    {   // Window
        auto menu = m_ui->menu_Window;
        fnAddAction(menu, CommandLeftSidebarWidgetToggle::Name);
        fnAddAction(menu, CommandMainWidgetToggleFullscreen::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandSwitchMainWidgetMode::Name);
        fnAddAction(menu, CommandPreviousDocument::Name);
        fnAddAction(menu, CommandNextDocument::Name);
    }

    {   // Help
        auto menu = m_ui->menu_Help;
        fnAddAction(menu, CommandReportBug::Name);
        fnAddAction(menu, CommandSystemInformation::Name);
        menu->addSeparator();
        fnAddAction(menu, CommandAbout::Name);
    }
}

void MainWindow::onOperationFinished(bool ok, const QString &msg)
{
    if (ok)
        WidgetMessageIndicator::showInfo(msg, this);
    else
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), msg);
}

void MainWindow::onGuiDocumentAdded(GuiDocument* guiDoc)
{
    auto gfxScene = guiDoc->graphicsScene();
    // Configure highlighting aspect
    auto fnConfigureHighlightStyle = [=](Prs3d_Drawer* drawer) {
        const QColor fillAreaQColor = mayoTheme()->color(Theme::Color::Graphic3d_AspectFillArea);
        if (!fillAreaQColor.isValid())
            return;

        auto fillArea = new Graphic3d_AspectFillArea3d;
        auto defaultShadingAspect = gfxScene->drawerDefault()->ShadingAspect();
        if (defaultShadingAspect && defaultShadingAspect->Aspect())
            *fillArea = *defaultShadingAspect->Aspect();

        const Quantity_Color fillAreaColor = QtGuiUtils::toPreferredColorSpace(fillAreaQColor);
        fillArea->SetInteriorColor(fillAreaColor);
        Graphic3d_MaterialAspect fillMaterial(Graphic3d_NOM_PLASTER);
        fillMaterial.SetColor(fillAreaColor);
        //fillMaterial.SetTransparency(0.1f);
        fillArea->SetFrontMaterial(fillMaterial);
        fillArea->SetBackMaterial(fillMaterial);
        drawer->SetDisplayMode(AIS_Shaded);
        drawer->SetBasicFillAreaAspect(fillArea);
    };
    fnConfigureHighlightStyle(gfxScene->drawerHighlight(Prs3d_TypeOfHighlight_LocalSelected).get());
    fnConfigureHighlightStyle(gfxScene->drawerHighlight(Prs3d_TypeOfHighlight_Selected).get());

    this->updateCurrentPage();
    this->updateControlsActivation();
}

void MainWindow::onGuiDocumentErased(GuiDocument* /*guiDoc*/)
{
    this->updateCurrentPage();
    this->updateControlsActivation();
}

void MainWindow::onMessage(MessageType msgType, const QString& text)
{
    switch (msgType) {
    case MessageType::Trace:
        qDebug() << text;
        break;
    case MessageType::Info:
        WidgetMessageIndicator::showInfo(text, this);
        break;
    case MessageType::Warning:
        QtWidgetsUtils::asyncMsgBoxWarning(this, tr("Warning"), text);
        break;
    case MessageType::Error:
        QtWidgetsUtils::asyncMsgBoxCritical(this, tr("Error"), text);
        break;
    }
}

void MainWindow::openDocumentsFromList(Span<const FilePath> listFilePath)
{
    FileCommandTools::openDocumentsFromList(m_appContext, listFilePath);
}

void MainWindow::updateControlsActivation()
{
    m_cmdContainer.foreachCommand([](std::string_view, Command* cmd) {
        cmd->action()->setEnabled(cmd->getEnabledStatus());
    });
}

void MainWindow::updateCurrentPage()
{
    const IAppContext::Page currentPage = m_appContext->currentPage();
    const bool appDocumentsEmpty = m_guiApp->guiDocuments().empty();
    const auto newPage = appDocumentsEmpty ? IAppContext::Page::Home : IAppContext::Page::Documents;
    if (currentPage != newPage)
        m_appContext->setCurrentPage(newPage);
}

IWidgetMainPage* MainWindow::widgetMainPage(IAppContext::Page page) const
{
    auto it = m_mapWidgetPage.find(page);
    return it != m_mapWidgetPage.cend() ? it->second : nullptr;
}

WidgetMainHome* MainWindow::widgetPageHome() const
{
    return dynamic_cast<WidgetMainHome*>(this->widgetMainPage(IAppContext::Page::Home));
}

WidgetMainControl* MainWindow::widgetPageDocuments() const
{
    return dynamic_cast<WidgetMainControl*>(this->widgetMainPage(IAppContext::Page::Documents));
}

} // namespace Mayo
