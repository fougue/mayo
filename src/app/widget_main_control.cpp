/****************************************************************************
** Copyright (c) 2023, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_main_control.h"
#include "ui_widget_main_control.h"

#include "../base/application.h"
#include "../graphics/graphics_utils.h"
#include "../gui/gui_application.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"
#include "../qtcommon/qtcore_utils.h"

#include "app_module.h"
#include "commands_api.h"
#include "commands_file.h"
#include "commands_window.h"
#include "document_files_watcher.h"
#include "document_property_group.h"
#include "gui_document_list_model.h"
#include "item_view_buttons.h"
#include "qtwidgets_utils.h"
#include "theme.h"
#include "widget_file_system.h"
#include "widget_gui_document.h"
#include "widget_model_tree.h"
#include "widget_occ_view.h"
#include "widget_properties_editor.h"

#include <QtCore/QDir>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <cassert>

namespace Mayo {

WidgetMainControl::WidgetMainControl(GuiApplication* guiApp, QWidget* parent)
    : IWidgetMainPage(parent),
      m_ui(new Ui_WidgetMainControl),
      m_guiApp(guiApp),
      m_docFilesWatcher(new DocumentFilesWatcher(guiApp->application(), this))
{
    assert(m_guiApp != nullptr);

    m_ui->setupUi(this);

    m_ui->widget_ModelTree->registerGuiApplication(guiApp);

    m_ui->splitter_Main->setChildrenCollapsible(false);
    m_ui->splitter_Main->setStretchFactor(0, 1);
    m_ui->splitter_Main->setStretchFactor(1, 3);

    m_ui->splitter_ModelTree->setStretchFactor(0, 1);
    m_ui->splitter_ModelTree->setStretchFactor(1, 2);

    m_ui->stack_LeftContents->setCurrentIndex(0);

    m_ui->widget_Properties->setRowHeightFactor(1.4);
    m_ui->widget_Properties->clear();

    mayoTheme()->setupHeaderComboBox(m_ui->combo_LeftContents);
    mayoTheme()->setupHeaderComboBox(m_ui->combo_GuiDocuments);

    // "Window" actions and navigation in documents
    QObject::connect(
                m_ui->combo_GuiDocuments, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &WidgetMainControl::onCurrentDocumentIndexChanged
    );
    QObject::connect(
                m_ui->widget_FileSystem, &WidgetFileSystem::locationActivated,
                this, &WidgetMainControl::onWidgetFileSystemLocationActivated
    );
    // ...
    QObject::connect(
                m_ui->combo_LeftContents, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &WidgetMainControl::onLeftContentsPageChanged
    );
    QObject::connect(
                m_ui->listView_OpenedDocuments, &QListView::clicked,
                this, [=](const QModelIndex& index) { this->setCurrentDocumentIndex(index.row()); }
    );

    guiApp->application()->signalDocumentFilePathChanged.connectSlot([=](const DocumentPtr& doc, const FilePath& fp) {
        if (this->currentWidgetGuiDocument()->documentIdentifier() == doc->identifier())
            m_ui->widget_FileSystem->setLocation(filepathTo<QFileInfo>(fp));
    });
    guiApp->selectionModel()->signalChanged.connectSlot(&WidgetMainControl::onApplicationItemSelectionChanged, this);
    guiApp->signalGuiDocumentAdded.connectSlot(&WidgetMainControl::onGuiDocumentAdded, this);

    // Document files monitoring
    auto appModule = AppModule::get();
    const auto& propReloadDocOnFileChange = appModule->properties()->reloadDocumentOnFileChange;
    m_docFilesWatcher->enable(propReloadDocOnFileChange);
    appModule->settings()->signalChanged.connectSlot([&](const Property* property) {
        if (property == &propReloadDocOnFileChange) {
            m_docFilesWatcher->enable(propReloadDocOnFileChange);
            m_pendingDocsToReload.clear();
        }
    });
    m_docFilesWatcher->signalDocumentFileChanged.connectSlot(&WidgetMainControl::onDocumentFileChanged, this);

    // Creation of annex objects
    m_listViewBtns = new ItemViewButtons(m_ui->listView_OpenedDocuments, this);
    m_listViewBtns->installDefaultItemDelegate();

    // BEWARE MainWindow::onGuiDocumentAdded() must be called before
    // MainWindow::onCurrentDocumentIndexChanged()
    auto guiDocModel = new GuiDocumentListModel(guiApp, this);
    m_ui->combo_GuiDocuments->setModel(guiDocModel);
    m_ui->listView_OpenedDocuments->setModel(guiDocModel);

    // Finalize setup
    m_ui->widget_LeftHeader->installEventFilter(this);
    m_ui->widget_ControlGuiDocuments->installEventFilter(this);
    m_ui->stack_GuiDocuments->installEventFilter(this);
    this->onLeftContentsPageChanged(m_ui->stack_LeftContents->currentIndex());
    m_ui->widget_MouseCoords->hide();

    this->onCurrentDocumentIndexChanged(-1);
}

WidgetMainControl::~WidgetMainControl()
{
    delete m_ui;
}

void WidgetMainControl::initialize(const CommandContainer* cmdContainer)
{
    assert(cmdContainer != nullptr);

    m_appContext = cmdContainer->appContext();
    auto fnFindAction = [=](std::string_view cmdName) {
        QAction* action = cmdContainer->findCommandAction(cmdName);
        assert(action != nullptr);
        return action;
    };
    m_ui->btn_PreviousGuiDocument->setDefaultAction(fnFindAction(CommandPreviousDocument::Name));
    m_ui->btn_NextGuiDocument->setDefaultAction(fnFindAction(CommandNextDocument::Name));
    m_ui->btn_CloseGuiDocument->setDefaultAction(fnFindAction(CommandCloseCurrentDocument::Name));
    m_ui->btn_CloseLeftSideBar->setDefaultAction(fnFindAction(CommandLeftSidebarWidgetToggle::Name));

    // Opened documents GUI
    auto actionCloseDoc = fnFindAction(CommandCloseCurrentDocument::Name);
    m_listViewBtns->addButton(1, actionCloseDoc->icon(), actionCloseDoc->toolTip());
    m_listViewBtns->setButtonDetection(1, -1, QVariant());
    m_listViewBtns->setButtonDisplayColumn(1, 0);
    m_listViewBtns->setButtonDisplayModes(1, ItemViewButtons::DisplayOnDetection);
    m_listViewBtns->setButtonItemSide(1, ItemViewButtons::ItemRightSide);
    const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
    m_listViewBtns->setButtonIconSize(1, QSize(iconSize * 0.66, iconSize * 0.66));
    QObject::connect(m_listViewBtns, &ItemViewButtons::buttonClicked, this, [=](int btnId, QModelIndex index) {
        if (btnId == 1) {
            assert(this->widgetGuiDocument(index.row()) != nullptr);
            FileCommandTools::closeDocument(
                        cmdContainer->appContext(),
                        this->widgetGuiDocument(index.row())->documentIdentifier()
            );
        }
    });
}

void WidgetMainControl::updatePageControlsActivation()
{
    const int appDocumentsCount = m_guiApp->application()->documentCount();
    const bool appDocumentsEmpty = appDocumentsCount == 0;
    m_ui->combo_GuiDocuments->setEnabled(!appDocumentsEmpty);
}

QWidget* WidgetMainControl::widgetLeftSideBar() const
{
    return m_ui->widget_Left;
}

bool WidgetMainControl::eventFilter(QObject* watched, QEvent* event)
{
    auto fnSizeBtn = [](const QWidget* container, const QWidget* widgetHeightRef) {
        const int btnSideLen = widgetHeightRef->frameGeometry().height();
        const QList<QAbstractButton*> listBtn = container->findChildren<QAbstractButton*>();
        for (QAbstractButton* btn : listBtn)
            btn->setFixedSize(btnSideLen, btnSideLen);
    };
    const QEvent::Type eventType = event->type();
    if (watched == m_ui->widget_ControlGuiDocuments && eventType == QEvent::Show) {
        fnSizeBtn(m_ui->widget_ControlGuiDocuments, m_ui->combo_GuiDocuments);
        return true;
    }

    if (watched == m_ui->widget_LeftHeader && eventType == QEvent::Show) {
        fnSizeBtn(m_ui->widget_LeftHeader, m_ui->combo_LeftContents);
        return true;
    }

    if (watched == m_ui->stack_GuiDocuments) {
        if (eventType == QEvent::Enter || eventType == QEvent::Leave) {
            m_ui->widget_MouseCoords->setHidden(eventType == QEvent::Leave);
            return true;
        }
    }

    return false;
}

QMenu* WidgetMainControl::createMenuModelTreeSettings()
{
    auto menu = new QMenu(this->findLeftHeaderPlaceHolder());
    menu->setToolTipsVisible(true);

    // Link with document selector
    auto appModule = AppModule::get();
    QAction* action = menu->addAction(to_QString(appModule->properties()->linkWithDocumentSelector.name().tr()));
    action->setCheckable(true);
    QObject::connect(action, &QAction::triggered, this, [=](bool on) {
        appModule->properties()->linkWithDocumentSelector.setValue(on);
    });

    // Model tree user actions
    menu->addSeparator();
    const WidgetModelTree_UserActions userActions = m_ui->widget_ModelTree->createUserActions(menu);
    for (QAction* usrAction : userActions.items)
        menu->addAction(usrAction);

    // Sync before menu show
    QObject::connect(menu, &QMenu::aboutToShow, this, [=]{
        action->setChecked(appModule->properties()->linkWithDocumentSelector);
        if (userActions.fnSyncItems)
            userActions.fnSyncItems();
    });

    return menu;
}

void WidgetMainControl::onApplicationItemSelectionChanged()
{
    WidgetModelTree* uiModelTree = m_ui->widget_ModelTree;
    WidgetPropertiesEditor* uiProps = m_ui->widget_Properties;

    uiProps->clear();
    Span<const ApplicationItem> spanAppItem = m_guiApp->selectionModel()->selectedItems();
    if (spanAppItem.size() == 1) {
        const ApplicationItem& appItem = spanAppItem.front();
        if (appItem.isDocument()) {
            auto dataProps = new DocumentPropertyGroup(appItem.document());
            uiProps->editProperties(dataProps, uiProps->addGroup(tr("Data")));
            m_ptrCurrentNodeDataProperties.reset(dataProps);
        }
        else if (appItem.isDocumentTreeNode()) {
            const DocumentTreeNode& docTreeNode = appItem.documentTreeNode();
            auto dataProps = AppModule::get()->properties(docTreeNode);
            if (dataProps) {
                uiProps->editProperties(dataProps.get(), uiProps->addGroup(tr("Data")));
                dataProps->signalPropertyChanged.connectSlot([=]{ uiModelTree->refreshItemText(appItem); });
                m_ptrCurrentNodeDataProperties = std::move(dataProps);
            }

            GuiDocument* guiDoc = m_guiApp->findGuiDocument(appItem.document());
            std::vector<GraphicsObjectPtr> vecGfxObject;
            guiDoc->foreachGraphicsObject(docTreeNode.id(), [&](GraphicsObjectPtr gfxObject) {
                vecGfxObject.push_back(std::move(gfxObject));
            });
            auto commonGfxDriver = GraphicsObjectDriver::getCommon(vecGfxObject);
            if (commonGfxDriver) {
                auto gfxProps = commonGfxDriver->properties(vecGfxObject);
                if (gfxProps) {
                    uiProps->editProperties(gfxProps.get(), uiProps->addGroup(tr("Graphics")));
                    gfxProps->signalPropertyChanged.connectSlot([=]{ guiDoc->graphicsScene()->redraw(); });
                    m_ptrCurrentNodeGraphicsProperties = std::move(gfxProps);
                }
            }
        }

        auto app = m_guiApp->application();
        if (AppModule::get()->properties()->linkWithDocumentSelector) {
            const int index = app->findIndexOfDocument(appItem.document());
            if (index != -1)
                this->setCurrentDocumentIndex(index);
        }
    }
    else {
        // TODO
        uiProps->clear();
    }

    emit this->updateGlobalControlsActivationRequired();
}


void WidgetMainControl::onLeftContentsPageChanged(int pageId)
{
    m_ui->stack_LeftContents->setCurrentIndex(pageId);
    QWidget* placeHolder = this->recreateLeftHeaderPlaceHolder();
    if (m_ui->stack_LeftContents->currentWidget() == m_ui->page_ModelTree && placeHolder) {
        const int btnSideLen = m_ui->combo_LeftContents->frameGeometry().height();
        auto btnSettings = new QToolButton(placeHolder);
        btnSettings->setAutoRaise(true);
        btnSettings->setFixedSize(btnSideLen, btnSideLen);
        btnSettings->setIcon(mayoTheme()->icon(Theme::Icon::Gear));
        btnSettings->setToolTip(tr("Options"));
        placeHolder->layout()->addWidget(btnSettings);
        btnSettings->setMenu(this->createMenuModelTreeSettings());
        btnSettings->setPopupMode(QToolButton::InstantPopup);
    }
    else {
        delete placeHolder;
    }
}

QWidget* WidgetMainControl::findLeftHeaderPlaceHolder() const
{
    return m_ui->widget_LeftHeader->findChild<QWidget*>(
                "LeftHeaderPlaceHolder", Qt::FindDirectChildrenOnly
    );
}

QWidget* WidgetMainControl::recreateLeftHeaderPlaceHolder()
{
    QWidget* placeHolder = this->findLeftHeaderPlaceHolder();
    delete placeHolder;
    placeHolder = new QWidget(m_ui->widget_LeftHeader);
    placeHolder->setObjectName("LeftHeaderPlaceHolder");
    auto layoutPlaceHolder = new QHBoxLayout(placeHolder);
    layoutPlaceHolder->setContentsMargins(0, 0, 0, 0);
    layoutPlaceHolder->setSpacing(0);
    m_ui->Layout_WidgetLeftHeader->insertWidget(2, placeHolder);
    return placeHolder;
}

void WidgetMainControl::reloadDocumentAfterChange(const DocumentPtr& doc)
{
    const QString strQuestion =
        tr("Document file `%1` has been changed since it was opened\n\n"
           "Do you want to reload that document?\n\n"
           "File: `%2`")
        .arg(to_QString(doc->name()))
        .arg(QDir::toNativeSeparators(filepathTo<QString>(doc->filePath())))
    ;
    const auto msgBtns = QMessageBox::Yes | QMessageBox::No;
    auto msgBox = new QMessageBox(QMessageBox::Question, tr("Question"), strQuestion, msgBtns, this);
    msgBox->setTextFormat(Qt::MarkdownText);
    QtWidgetsUtils::asyncDialogExec(msgBox);
    QObject::connect(msgBox, &QMessageBox::buttonClicked, this, [=](QAbstractButton* btn) {
        m_docFilesWatcher->acknowledgeDocumentFileChange(doc);
        if (btn == msgBox->button(QMessageBox::Yes)) {
            while (doc->entityCount() > 0)
                doc->destroyEntity(doc->entityTreeNodeId(0));

            FileCommandTools::importInDocument(m_appContext, doc, doc->filePath());
        }
    });
}

WidgetGuiDocument* WidgetMainControl::widgetGuiDocument(int idx) const
{
    return qobject_cast<WidgetGuiDocument*>(m_ui->stack_GuiDocuments->widget(idx));
}

WidgetGuiDocument* WidgetMainControl::currentWidgetGuiDocument() const
{
    return this->widgetGuiDocument(this->currentDocumentIndex());
}

int WidgetMainControl::indexOfWidgetGuiDocument(WidgetGuiDocument* widgetDoc) const
{
    return m_ui->stack_GuiDocuments->indexOf(widgetDoc);
}

void WidgetMainControl::removeWidgetGuiDocument(WidgetGuiDocument* widgetDoc)
{
    if (widgetDoc) {
        m_ui->stack_GuiDocuments->removeWidget(widgetDoc);
        widgetDoc->deleteLater();
    }
}

int WidgetMainControl::widgetGuiDocumentCount() const
{
    return m_ui->stack_GuiDocuments->count();
}

void WidgetMainControl::onGuiDocumentAdded(GuiDocument* guiDoc)
{
    auto gfxScene = guiDoc->graphicsScene();

    // Configure 3D view behavior with respect to application settings
    auto appModule = AppModule::get();
    auto appProps = appModule->properties();
    auto widget = new WidgetGuiDocument(guiDoc);
    guiDoc->setDevicePixelRatio(widget->devicePixelRatioF());
    auto widgetCtrl = widget->controller();
    widgetCtrl->setInstantZoomFactor(appProps->instantZoomFactor);
    widgetCtrl->setNavigationStyle(appProps->navigationStyle);
    if (appProps->defaultShowOriginTrihedron) {
        guiDoc->toggleOriginTrihedronVisibility();
        gfxScene->redraw();
    }

    appModule->settings()->signalChanged.connectSlot([=](const Property* setting) {
        if (setting == &appProps->instantZoomFactor)
            widgetCtrl->setInstantZoomFactor(appProps->instantZoomFactor);
        else if (setting == &appProps->navigationStyle)
            widgetCtrl->setNavigationStyle(appProps->navigationStyle);
    });

    // React to mouse move in 3D view:
    //   * update highlighting
    //   * compute and display 3D mouse coordinates(by silent picking)
    widgetCtrl->signalMouseMoved.connectSlot([=](int xPos, int yPos) {
        const double dpRatio = this->devicePixelRatioF();
        gfxScene->highlightAt(xPos * dpRatio, yPos * dpRatio, guiDoc->v3dView());
        widget->view()->redraw();
        auto selector = gfxScene->mainSelector();
        selector->Pick(xPos, yPos, guiDoc->v3dView());
        const gp_Pnt pos3d =
                selector->NbPicked() > 0 ?
                    selector->PickedPoint(1) :
                    GraphicsUtils::V3dView_to3dPosition(guiDoc->v3dView(), xPos, yPos);
        m_ui->label_ValuePosX->setText(QString::number(pos3d.X(), 'f', 3));
        m_ui->label_ValuePosY->setText(QString::number(pos3d.Y(), 'f', 3));
        m_ui->label_ValuePosZ->setText(QString::number(pos3d.Z(), 'f', 3));
    });

    m_ui->stack_GuiDocuments->addWidget(widget);
    const int newDocIndex = m_guiApp->application()->documentCount() - 1;
    QtCoreUtils::runJobOnMainThread([=]{ this->setCurrentDocumentIndex(newDocIndex); });
}

int WidgetMainControl::currentDocumentIndex() const
{
    return m_ui->combo_GuiDocuments->currentIndex();
}

void WidgetMainControl::setCurrentDocumentIndex(int idx)
{
    m_ui->combo_GuiDocuments->setCurrentIndex(idx);
}

void WidgetMainControl::onWidgetFileSystemLocationActivated(const QFileInfo& loc)
{
    FileCommandTools::openDocument(m_appContext, filepathFrom(loc));
}

void WidgetMainControl::onCurrentDocumentIndexChanged(int idx)
{
    m_ui->stack_GuiDocuments->setCurrentIndex(idx);
    QAbstractItemView* view = m_ui->listView_OpenedDocuments;
    view->setCurrentIndex(view->model()->index(idx, 0));

    emit this->updateGlobalControlsActivationRequired();

    const DocumentPtr docPtr = m_guiApp->application()->findDocumentByIndex(idx);
    const FilePath docFilePath = docPtr ? docPtr->filePath() : FilePath();
    m_ui->widget_FileSystem->setLocation(filepathTo<QFileInfo>(docFilePath));

    if (m_docFilesWatcher->isEnabled()) {
        auto itDoc = m_pendingDocsToReload.find(docPtr);
        if (itDoc != m_pendingDocsToReload.end()) {
            m_pendingDocsToReload.erase(itDoc);
            this->reloadDocumentAfterChange(docPtr);
        }
    }

    emit this->currentDocumentIndexChanged(idx);
}

void WidgetMainControl::onDocumentFileChanged(const DocumentPtr& doc)
{
    WidgetGuiDocument* widgetDoc = nullptr;
    for (int i = 0; i < this->widgetGuiDocumentCount() && !widgetDoc; ++i) {
        const DocumentPtr& candidateDoc = this->widgetGuiDocument(i)->guiDocument()->document();
        if (candidateDoc->identifier() == doc->identifier())
            widgetDoc = this->widgetGuiDocument(i);
    }

    if (!widgetDoc)
        return;

    if (widgetDoc == this->currentWidgetGuiDocument())
        this->reloadDocumentAfterChange(doc);
    else
        m_pendingDocsToReload.insert(doc);
}

} // namespace Mayo
