/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "application.h"
#include "dialog_about.h"
#include "dialog_export_options.h"
#include "dialog_inspect_xde.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "dialog_task_manager.h"
#include "document.h"
#include "gpx_utils.h"
#include "gui_application.h"
#include "gui_document.h"
#include "qt_occ_view_controller.h"
#include "theme.h"
#include "widget_gui_document.h"
#include "widget_message_indicator.h"
#include "xde_document_item.h"
#include "fougtools/qttools/gui/item_view_buttons.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/runner_stdasync.h"

#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtGui/QDesktopServices>
#include <QtCore/QStringListModel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFileSystemModel>

namespace Mayo {

namespace Internal {

static const QLatin1String keyLastOpenDir("GUI/MainWindow_lastOpenDir");
static const QLatin1String keyLastSelectedFilter("GUI/MainWindow_lastSelectedFilter");

class DocumentModel : public QStringListModel {
public:
    DocumentModel(Application* app)
        : QStringListModel(app)
    {
        for (Document* doc : app->documents())
            this->appendDocument(doc);
        QObject::connect(
                    app, &Application::documentAdded,
                    this, &DocumentModel::appendDocument);
        QObject::connect(
                    app, &Application::documentErased,
                    this, &DocumentModel::removeDocument);
    }

private:
    void appendDocument(Document* doc)
    {
        const int rowId = this->rowCount();
        this->insertRow(rowId);
        this->setData(this->index(rowId), doc->label());
        m_docs.emplace_back(doc);
    }

    void removeDocument(const Document* doc)
    {
        auto itFound = std::find(m_docs.begin(), m_docs.end(), doc);
        if (itFound != m_docs.end()) {
            this->removeRow(itFound - m_docs.begin());
            m_docs.erase(itFound);
        }
    }

    std::vector<const Document*> m_docs;
};

static Application::PartFormat partFormatFromFilter(const QString& filter)
{
    const auto itFormatFound =
            std::find_if(Application::partFormats().cbegin(),
                         Application::partFormats().cend(),
                         [=](Application::PartFormat format) {
        return filter == Application::partFormatFilter(format);
    } );
    return itFormatFound != Application::partFormats().cend() ?
                *itFormatFound :
                Application::PartFormat::Unknown;
}

struct ImportExportSettings {
    QString openDir;
    QString selectedFilter;

    static ImportExportSettings load()
    {
        QSettings settings;
        return { settings.value(keyLastOpenDir, QString()).toString(),
                 settings.value(keyLastSelectedFilter, QString()).toString() };
    }

    static void save(const ImportExportSettings& sets)
    {
        QSettings settings;
        settings.setValue(keyLastOpenDir, sets.openDir);
        settings.setValue(keyLastSelectedFilter, sets.selectedFilter);
    }
};

struct OpenFileNames {
    QStringList listFilepath;
    ImportExportSettings lastIoSettings;
    Application::PartFormat selectedFormat;

    enum GetOption {
        GetOne,
        GetMany
    };

    static OpenFileNames get(
            QWidget* parentWidget,
            OpenFileNames::GetOption option = OpenFileNames::GetMany)
    {
        OpenFileNames result;
        result.selectedFormat = Application::PartFormat::Unknown;
        result.lastIoSettings = ImportExportSettings::load();
        QStringList listPartFormatFilter = Application::partFormatFilters();
        const QString allFilesFilter = Application::tr("All files(*.*)");
        listPartFormatFilter.append(allFilesFilter);
        const QString dlgTitle = Application::tr("Select Part File");
        const QString& dlgOpenDir = result.lastIoSettings.openDir;
        const QString dlgFilter = listPartFormatFilter.join(QLatin1String(";;"));
        QString* dlgPtrSelFilter = &result.lastIoSettings.selectedFilter;
        if (option == OpenFileNames::GetOne) {
            const QString filepath =
                    QFileDialog::getOpenFileName(
                        parentWidget, dlgTitle, dlgOpenDir, dlgFilter, dlgPtrSelFilter);
            result.listFilepath.clear();
            result.listFilepath.push_back(filepath);
        }
        else {
            result.listFilepath =
                    QFileDialog::getOpenFileNames(
                        parentWidget, dlgTitle, dlgOpenDir, dlgFilter, dlgPtrSelFilter);
        }
        if (!result.listFilepath.isEmpty()) {
            result.lastIoSettings.openDir =
                    QFileInfo(result.listFilepath.front()).canonicalPath();
            result.selectedFormat =
                    result.lastIoSettings.selectedFilter != allFilesFilter ?
                        partFormatFromFilter(result.lastIoSettings.selectedFilter) :
                        Application::PartFormat::Unknown;
        }
        return result;
    }
};

static gp_Pnt pointUnderMouse(const GuiDocument* guiDoc, const QPoint& pos)
{
    const Handle_AIS_InteractiveContext& ctx = guiDoc->aisInteractiveContext();
    ctx->MoveTo(pos.x(), pos.y(), guiDoc->v3dView(), true);
    if (ctx->HasDetected() && ctx->MainSelector()->NbPicked() > 0)
        return ctx->MainSelector()->PickedPoint(1);
    return GpxUtils::V3dView_to3dPosition(guiDoc->v3dView(), pos.x(), pos.y());
}

} // namespace Internal

MainWindow::MainWindow(GuiApplication *guiApp, QWidget *parent)
    : QMainWindow(parent),
      m_guiApp(guiApp),
      m_ui(new Ui_MainWindow)
{
    m_ui->setupUi(this);
    m_ui->centralWidget->setStyleSheet(
                "QSplitter::handle:vertical { width: 2px; }\n"
                "QSplitter::handle:horizontal { width: 2px; }\n");
    m_ui->splitter_Main->setChildrenCollapsible(false);
    m_ui->splitter_Main->setStretchFactor(0, 1);
    m_ui->splitter_Main->setStretchFactor(1, 3);

    m_ui->splitter_ApplicationTree->setStretchFactor(0, 2);
    m_ui->splitter_ApplicationTree->setStretchFactor(1, 1);

    m_ui->stack_LeftContents->setCurrentIndex(0);

    m_ui->widget_DocumentProps->setGuiApplication(guiApp);
    m_ui->widget_DocumentProps->editDocumentItems(std::vector<DocumentItem*>());

    m_ui->btn_PreviousGuiDocument->setDefaultAction(m_ui->actionPreviousDoc);
    m_ui->btn_NextGuiDocument->setDefaultAction(m_ui->actionNextDoc);
    m_ui->btn_CloseGuiDocument->setDefaultAction(m_ui->actionCloseDoc);

    // Style sheet for the combo boxes just below the menu bar
    {
        const QString comboStyleSheet = QString(
                "QComboBox {"
                "    border-style: solid;"
                "    background: %1;"
                "    padding: 2px 15px 2px 10px;"
                "}\n"
                "QComboBox:hover {"
                "    border-style: solid;"
                "    background: %2;"
                "    padding: 2px 15px 2px 10px;"
                "}\n"
                "QComboBox::drop-down {"
                "    subcontrol-origin: padding;"
                "    subcontrol-position: top right;"
                "    width: 15px;"
                "    border-left-width: 0px;"
                "    border-top-right-radius: 3px;"
                "    border-bottom-right-radius: 3px;"
                "}\n"
                "QComboBox::down-arrow { image: url(%3); }\n"
                "QComboBox::down-arrow:disabled { image: url(%4); }\n"
                ).arg(mayoTheme()->color(Theme::Color::FlatBackground).name(),
                      mayoTheme()->color(Theme::Color::FlatHover).name(),
                      mayoTheme()->imageUrl(Theme::Image::FlatDownIndicator),
                      mayoTheme()->imageUrl(Theme::Image::FlatDownIndicatorDisabled));
        m_ui->combo_GuiDocuments->setStyleSheet(comboStyleSheet);
        m_ui->combo_LeftContents->setStyleSheet(comboStyleSheet);
    }

    // Opened documents GUI
    {
        auto listViewBtns =
                new qtgui::ItemViewButtons(m_ui->listView_OpenedDocuments, this);
        listViewBtns->addButton(
                    1, QPixmap(":/images/close.png"), m_ui->actionCloseDoc->toolTip());
        listViewBtns->setButtonDetection(1, -1, QVariant());
        listViewBtns->setButtonDisplayColumn(1, 0);
        listViewBtns->setButtonDisplayModes(
                    1, qtgui::ItemViewButtons::DisplayOnDetection);
        listViewBtns->setButtonItemSide(
                    1, qtgui::ItemViewButtons::ItemRightSide);
        const int iconSize = this->style()->pixelMetric(QStyle::PM_ListViewIconSize);
        listViewBtns->setButtonIconSize(1, QSize(iconSize * 0.66, iconSize * 0.66));
        listViewBtns->installDefaultItemDelegate();
        QObject::connect(
                    listViewBtns, &qtgui::ItemViewButtons::buttonClicked,
                    [=](int btnId, const QModelIndex& index) {
            if (btnId == 1)
                this->closeDocument(index.row());
        });
    }

    new DialogTaskManager(this);

    m_fileSysModel = new QFileSystemModel(this);
    m_ui->treeView_FileSystem->setModel(m_fileSysModel);

    auto docModel = new Internal::DocumentModel(Application::instance());
    m_ui->combo_GuiDocuments->setModel(docModel);
    m_ui->listView_OpenedDocuments->setModel(docModel);

    const auto sigComboIndexChanged =
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
    // "File" actions
    QObject::connect(
                m_ui->actionNewDoc, &QAction::triggered,
                this, &MainWindow::newDoc);
    QObject::connect(
                m_ui->actionOpen, &QAction::triggered,
                this, &MainWindow::openPartInNewDoc);
    QObject::connect(
                m_ui->actionImport, &QAction::triggered,
                this, &MainWindow::importInCurrentDoc);
    QObject::connect(
                m_ui->actionExportSelectedItems, &QAction::triggered,
                this, &MainWindow::exportSelectedItems);
    QObject::connect(
                m_ui->actionQuit, &QAction::triggered,
                this, &MainWindow::quitApp);
    QObject::connect(
                m_ui->label_MainHome, &QLabel::linkActivated,
                this, &MainWindow::onHomePageLinkActivated);
    // "Tools" actions
    QObject::connect(
                m_ui->actionSaveImageView, &QAction::triggered,
                this, &MainWindow::saveImageView);
    QObject::connect(
                m_ui->actionInspectXDE, &QAction::triggered,
                this, &MainWindow::inspectXde);
    QObject::connect(
                m_ui->actionOptions, &QAction::triggered,
                this, &MainWindow::editOptions);
    // "Help" actions
    QObject::connect(
                m_ui->actionReportBug, &QAction::triggered,
                this, &MainWindow::reportbug);
    QObject::connect(
                m_ui->actionAboutMayo, &QAction::triggered,
                this, &MainWindow::aboutMayo);
    // "Window" actions and navigation in documents
    QObject::connect(
                m_ui->actionFullscreenOrNormal, &QAction::triggered,
                this, &MainWindow::toggleFullscreen);
    QObject::connect(
                m_ui->actionShowHideLeftSidebar, &QAction::triggered,
                this, &MainWindow::toggleLeftSidebar);
    QObject::connect(m_ui->actionPreviousDoc, &QAction::triggered, [=]{
        this->setCurrentDocumentIndex(this->currentDocumentIndex() - 1);
    });
    QObject::connect(m_ui->actionNextDoc, &QAction::triggered, [=]{
        this->setCurrentDocumentIndex(this->currentDocumentIndex() + 1);
    });
    QObject::connect(
                m_ui->actionCloseDoc, &QAction::triggered,
                this, &MainWindow::closeCurrentDocument);
    QObject::connect(
                m_ui->combo_GuiDocuments, sigComboIndexChanged,
                this, &MainWindow::currentDocumentIndexChanged);
    QObject::connect(
                this, &MainWindow::currentDocumentIndexChanged,
                m_ui->stack_GuiDocuments, &QStackedWidget::setCurrentIndex);
    QObject::connect(
                this, &MainWindow::currentDocumentIndexChanged,
                this, &MainWindow::updateControlsActivation);
    // ...
    QObject::connect(
                m_ui->combo_LeftContents, sigComboIndexChanged,
                m_ui->stack_LeftContents, &QStackedWidget::setCurrentIndex);
    QObject::connect(
                guiApp, &GuiApplication::guiDocumentAdded,
                this, &MainWindow::onGuiDocumentAdded);
    QObject::connect(
                m_ui->widget_ApplicationTree, &WidgetApplicationTree::selectionChanged,
                this, &MainWindow::onApplicationTreeWidgetSelectionChanged);
    QObject::connect(
                m_ui->listView_OpenedDocuments, &QListView::clicked,
                [=](const QModelIndex& index) {
        this->setCurrentDocumentIndex(index.row());
    });
    QObject::connect(
                this, &MainWindow::currentDocumentIndexChanged, [=](int docId) {
        QAbstractItemView* view = m_ui->listView_OpenedDocuments;
        view->setCurrentIndex(view->model()->index(docId, 0));
    });
    QObject::connect(
                this, &MainWindow::operationFinished,
                this, &MainWindow::onOperationFinished);

    m_ui->widget_ControlGuiDocuments->installEventFilter(this);
    this->updateControlsActivation();
    this->updateActionText(m_ui->actionFullscreenOrNormal);
    this->updateActionText(m_ui->actionShowHideLeftSidebar);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_ui->widget_ControlGuiDocuments
            && event->type() == QEvent::Show)
    {
        const int btnSideLen = m_ui->combo_GuiDocuments->frameGeometry().height();
        const QList<ButtonFlat*> listBtn =
                m_ui->widget_ControlGuiDocuments->findChildren<ButtonFlat*>();
        for (ButtonFlat* btn : listBtn)
            btn->setFixedSize(btnSideLen, btnSideLen);
        return true;
    }
    return false;
}

void MainWindow::newDoc()
{
    Application::instance()->addDocument();
}

void MainWindow::openPartInNewDoc()
{
    this->foreachOpenFileName(
                [=](Application::PartFormat format, QString filepath) {
        const QString filename = QFileInfo(filepath).fileName();
        Document* doc = Application::instance()->addDocument(filename);
        m_fileSysModel->setRootPath(filepath);
        this->runImportTask(doc, format, filepath);
    });
}

void MainWindow::importInCurrentDoc()
{
    auto docView3d =
            qobject_cast<WidgetGuiDocument*>(
                m_ui->stack_GuiDocuments->currentWidget());
    if (docView3d != nullptr) {
        Document* doc = docView3d->guiDocument()->document();
        this->foreachOpenFileName(
                    [=](Application::PartFormat format, QString filepath) {
            this->runImportTask(doc, format, filepath);
        });
    }
}

void MainWindow::runImportTask(
        Document* doc, Application::PartFormat format, const QString& filepath)
{
    auto task = qttask::Manager::globalInstance()->newTask<qttask::StdAsync>();
    task->run([=]{
        QTime chrono;
        chrono.start();
        const Application::IoResult result =
                Application::instance()->importInDocument(
                    doc, format, filepath, &task->progress());
        QString msg;
        if (result.ok) {
            msg = tr("Import time '%1': %2ms")
                    .arg(QFileInfo(filepath).fileName())
                    .arg(chrono.elapsed());
        } else {
            msg = tr("Failed to import part:\n    %1\nError: %2")
                    .arg(filepath, result.errorText);
        }
        emit operationFinished(result.ok, msg);
    });
}

void MainWindow::runExportTask(
        const std::vector<DocumentItem*>& docItems,
        Application::PartFormat format,
        const Application::ExportOptions& opts,
        const QString& filepath)
{
    auto task = qttask::Manager::globalInstance()->newTask<qttask::StdAsync>();
    task->run([=]{
        QTime chrono;
        chrono.start();
        const Application::IoResult result =
                Application::instance()->exportDocumentItems(
                    docItems, format, opts, filepath, &task->progress());
        QString msg;
        if (result.ok) {
            msg = tr("Export time '%1': %2ms")
                    .arg(QFileInfo(filepath).fileName())
                    .arg(chrono.elapsed());
        } else {
            msg = tr("Failed to export part:\n    %1\nError: %2")
                    .arg(filepath).arg(result.errorText);
        }
        emit operationFinished(result.ok, msg);
    });
}

void MainWindow::exportSelectedItems()
{
    static const Application::ExportOptions defaultOpts;
    auto lastSettings = Internal::ImportExportSettings::load();
    const QString filepath =
            QFileDialog::getSaveFileName(
                this,
                tr("Select Output File"),
                lastSettings.openDir,
                Application::partFormatFilters().join(QLatin1String(";;")),
                &lastSettings.selectedFilter);
    if (!filepath.isEmpty()) {
        lastSettings.openDir = QFileInfo(filepath).canonicalPath();
        const Application::PartFormat format =
                Internal::partFormatFromFilter(lastSettings.selectedFilter);
        const std::vector<DocumentItem*> vecDocItem =
                m_ui->widget_ApplicationTree->selectedDocumentItems();
        if (Application::hasExportOptionsForFormat(format)) {
#ifdef HAVE_GMIO
            auto dlg = new DialogExportOptions(this);
            dlg->setPartFormat(format);
            QObject::connect(dlg, &QDialog::accepted, [=]{
                const Application::ExportOptions opts = dlg->currentExportOptions();
                this->runExportTask(vecDocItem, format, opts, filepath);
                Internal::ImportExportSettings::save(lastSettings);
            });
            qtgui::QWidgetUtils::asyncDialogExec(dlg);
#else
            this->runExportTask(vecDocItem, format, defaultOpts, filepath);
            Internal::ImportExportSettings::save(lastSettings);
#endif
        }
        else {
            this->runExportTask(vecDocItem, format, defaultOpts, filepath);
            Internal::ImportExportSettings::save(lastSettings);
        }
    }
}

void MainWindow::quitApp()
{
    QApplication::quit();
}

void MainWindow::editOptions()
{
    auto dlg = new DialogOptions(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::saveImageView()
{
    auto widget =
            qobject_cast<const WidgetGuiDocument*>(
                m_ui->stack_GuiDocuments->currentWidget());
    auto dlg = new DialogSaveImageView(widget->guiDocument()->v3dView());
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::inspectXde()
{
    const std::vector<DocumentItem*> vecDocItem =
            m_ui->widget_ApplicationTree->selectedDocumentItems();
    const XdeDocumentItem* xdeDocItem = nullptr;
    for (const DocumentItem* docItem : vecDocItem) {
        xdeDocItem = dynamic_cast<const XdeDocumentItem*>(docItem);
        if (xdeDocItem != nullptr)
            break;
    }
    if (xdeDocItem != nullptr) {
        auto dlg = new DialogInspectXde(this);
        dlg->load(xdeDocItem->cafDoc());
        qtgui::QWidgetUtils::asyncDialogExec(dlg);
    }
}

void MainWindow::toggleFullscreen()
{
    if (this->isFullScreen()) {
        if (m_previousWindowState.testFlag(Qt::WindowMaximized))
            this->showMaximized();
        else
            this->showNormal();
    }
    else {
        m_previousWindowState = this->windowState();
        this->showFullScreen();
    }
    this->updateActionText(m_ui->actionFullscreenOrNormal);
}

void MainWindow::toggleLeftSidebar()
{
    const bool isLeftSideBarVisible = m_ui->widget_Left->isVisible();
    m_ui->widget_Left->setVisible(!isLeftSideBarVisible);
    this->updateActionText(m_ui->actionShowHideLeftSidebar);
}

void MainWindow::aboutMayo()
{
    auto dlg = new DialogAbout(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::reportbug()
{
    QDesktopServices::openUrl(
                QUrl(QStringLiteral("https://github.com/fougue/mayo/issues")));
}

void MainWindow::onApplicationTreeWidgetSelectionChanged()
{
    const std::vector<DocumentItem*> vecDocItem =
            m_ui->widget_ApplicationTree->selectedDocumentItems();
    if (vecDocItem.size() >= 1) {
        m_ui->widget_DocumentProps->editDocumentItems(vecDocItem);

    }
    else {
        const std::vector<HandleProperty> vecHndProp =
                m_ui->widget_ApplicationTree->propertiesOfCurrentObject();
        m_ui->widget_DocumentProps->editProperties(vecHndProp);
    }
}

void MainWindow::onOperationFinished(bool ok, const QString &msg)
{
    if (ok)
        WidgetMessageIndicator::showMessage(msg, this);
    else
        qtgui::QWidgetUtils::asyncMsgBoxCritical(this, tr("Error"), msg);
}

void MainWindow::onHomePageLinkActivated(const QString &link)
{
    if (link == "NewDocument")
        m_ui->actionNewDoc->trigger();
    else if (link == "OpenDocuments")
        m_ui->actionOpen->trigger();
}

void MainWindow::onGuiDocumentAdded(GuiDocument* guiDoc)
{
    auto widget = new WidgetGuiDocument(guiDoc);
    BaseV3dViewController* ctrl = widget->controller();
    QObject::connect(
                ctrl, &BaseV3dViewController::mouseMoved,
                [=](const QPoint& pos2d) {
        if (!ctrl->isPanning() && !ctrl->isRotating()) {
            const gp_Pnt pos3d = Internal::pointUnderMouse(guiDoc, pos2d);
            m_ui->label_ValuePosX->setText(QString::number(pos3d.X(), 'f', 3));
            m_ui->label_ValuePosY->setText(QString::number(pos3d.Y(), 'f', 3));
            m_ui->label_ValuePosZ->setText(QString::number(pos3d.Z(), 'f', 3));
        }
    });
    m_ui->stack_GuiDocuments->addWidget(widget);
    this->updateControlsActivation();
}

void MainWindow::closeCurrentDocument()
{
    this->closeDocument(this->currentDocumentIndex());
}

void MainWindow::closeDocument(int docIndex)
{
    if (0 <= docIndex && docIndex < m_ui->stack_GuiDocuments->count()) {
        QWidget* tab = m_ui->stack_GuiDocuments->widget(docIndex);
        auto docView3d = qobject_cast<WidgetGuiDocument*>(tab);
        Application::instance()->eraseDocument(docView3d->guiDocument()->document());
        m_ui->stack_GuiDocuments->removeWidget(docView3d);
        this->updateControlsActivation();
    }
}

void MainWindow::foreachOpenFileName(
        std::function<void (Application::PartFormat, QString)>&& func)
{
    const auto resFileNames = Internal::OpenFileNames::get(this);
    if (!resFileNames.listFilepath.isEmpty()) {
        for (const QString& filepath : resFileNames.listFilepath) {
            const Application::PartFormat fileFormat =
                    resFileNames.selectedFormat != Application::PartFormat::Unknown ?
                        resFileNames.selectedFormat :
                        Application::findPartFormat(filepath);
            if (fileFormat != Application::PartFormat::Unknown) {
                func(fileFormat, filepath);
            }
            else {
                qtgui::QWidgetUtils::asyncMsgBoxCritical(
                            this,
                            tr("Error"),
                            tr("'%1'\nUnknown file format").arg(filepath));
            }
        }
        Internal::ImportExportSettings::save(resFileNames.lastIoSettings);
    }
}

void MainWindow::updateControlsActivation()
{
    const QWidget* currMainPage = m_ui->stack_Main->currentWidget();
    const size_t appDocumentsCount = Application::instance()->documents().size();
    const bool appDocumentsEmpty = appDocumentsCount == 0;
    QWidget* newMainPage =
            appDocumentsEmpty ? m_ui->page_MainHome : m_ui->page_MainControl;
    if (currMainPage != newMainPage)
        m_ui->stack_Main->setCurrentWidget(newMainPage);
    m_ui->actionImport->setEnabled(!appDocumentsEmpty);
    m_ui->actionSaveImageView->setEnabled(!appDocumentsEmpty);
    m_ui->actionCloseDoc->setEnabled(!appDocumentsEmpty);
    const int currentDocIndex = this->currentDocumentIndex();
    m_ui->actionPreviousDoc->setEnabled(
                !appDocumentsEmpty && currentDocIndex > 0);
    m_ui->actionNextDoc->setEnabled(
                !appDocumentsEmpty && currentDocIndex < appDocumentsCount - 1);
    m_ui->actionExportSelectedItems->setEnabled(!appDocumentsEmpty);
    m_ui->actionShowHideLeftSidebar->setEnabled(newMainPage != m_ui->page_MainHome);
    m_ui->combo_GuiDocuments->setEnabled(!appDocumentsEmpty);
}

void MainWindow::updateActionText(QAction* action)
{
    QString text;
    if (action == m_ui->actionFullscreenOrNormal) {
        text = this->isFullScreen() ? tr("Normal") : tr("Fullscreen");
    }
    else if (action == m_ui->actionShowHideLeftSidebar) {
        text = m_ui->widget_Left->isVisible() ?
                    tr("Hide Left Sidebar") :
                    tr("Show Left Sidebar");
    }
    if (!text.isEmpty())
        action->setText(text);
}

int MainWindow::currentDocumentIndex() const
{
    return m_ui->combo_GuiDocuments->currentIndex();
}

void MainWindow::setCurrentDocumentIndex(int idx)
{
    m_ui->combo_GuiDocuments->setCurrentIndex(idx);
}

} // namespace Mayo
