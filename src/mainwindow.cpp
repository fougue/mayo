/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "application.h"
#include "application_item_selection_model.h"
#include "brep_utils.h"
#include "dialog_about.h"
#include "dialog_export_options.h"
#include "dialog_inspect_xde.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "dialog_task_manager.h"
#include "document.h"
#include "document_item.h"
#include "gui_document_list_model.h"
#include "gpx_utils.h"
#include "gui_application.h"
#include "gui_document.h"
#include "options.h"
#include "qt_occ_view_controller.h"
#include "theme.h"
#include "widget_application_tree.h"
#include "widget_file_system.h"
#include "widget_gui_document.h"
#include "widget_properties_editor.h"
#include "widget_message_indicator.h"
#include "xde_document_item.h"
#include "xde_shape_property_owner.h"

#include <fougtools/qttools/gui/item_view_buttons.h>
#include <fougtools/qttools/gui/qwidget_utils.h>
#include <fougtools/qttools/task/manager.h>
#include <fougtools/qttools/task/runner_stdasync.h>

#include <QtCore/QMimeData>
#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtCore/QStringListModel>
#include <QtGui/QDesktopServices>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QWidgetAction>
#include <QtDebug>
#include <tuple>

namespace Mayo {

namespace Internal {

static const char keyLastOpenDir[] = "GUI/MainWindow_lastOpenDir";
static const char keyLastSelectedFilter[] = "GUI/MainWindow_lastSelectedFilter";
static const char keyLinkWithDocumentSelector[] = "GUI/MainWindow_LinkWithDocumentSelector";
static const char keyReferenceItemTextTemplateId[] = "GUI/MainWindow_ReferenceItemTextTemplateId";

static Application::PartFormat partFormatFromFilter(const QString& filter)
{
    for (Application::PartFormat format : Application::partFormats()) {
        if (filter == Application::partFormatFilter(format))
            return format;
    }
    return Application::PartFormat::Unknown;
}

// TODO: move in Options
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
            ImportExportSettings::save(result.lastIoSettings);
        }
        return result;
    }
};

static void msgBoxErrorFileFormat(QWidget* parent, const QString& filepath)
{
    qtgui::QWidgetUtils::asyncMsgBoxCritical(
                parent,
                MainWindow::tr("Error"),
                MainWindow::tr("'%1'\nUnknown file format").arg(filepath));
}

static void prependRecentFile(QStringList* listRecentFile, const QString& filepath)
{
    constexpr int sizeLimit = 10;
    const QString absFilepath =
            QDir::toNativeSeparators(QFileInfo(filepath).absoluteFilePath());
    for (const QString& recentFile : *listRecentFile) {
        if (recentFile == absFilepath)
            return;
    }
    listRecentFile->insert(listRecentFile->begin(), absFilepath);
    while (listRecentFile->size() > sizeLimit)
        listRecentFile->pop_back();
}

} // namespace Internal

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui_MainWindow),
      m_listRecentFile(Options::instance()->recentFiles())
{
    m_ui->setupUi(this);
    m_ui->splitter_Main->setChildrenCollapsible(false);
    m_ui->splitter_Main->setStretchFactor(0, 1);
    m_ui->splitter_Main->setStretchFactor(1, 3);

    m_ui->splitter_ApplicationTree->setStretchFactor(0, 1);
    m_ui->splitter_ApplicationTree->setStretchFactor(1, 2);

    m_ui->stack_LeftContents->setCurrentIndex(0);

    m_ui->widget_Properties->setRowHeightFactor(1.4);
    m_ui->widget_Properties->clear();

    m_ui->btn_PreviousGuiDocument->setDefaultAction(m_ui->actionPreviousDoc);
    m_ui->btn_NextGuiDocument->setDefaultAction(m_ui->actionNextDoc);
    m_ui->btn_CloseGuiDocument->setDefaultAction(m_ui->actionCloseDoc);

    m_ui->actionAboutMayo->setText(tr("About %1").arg(QApplication::applicationName()));
    m_ui->actionImport->setIcon(mayoTheme()->icon(Theme::Icon::Import));
    m_ui->actionExportSelectedItems->setIcon(mayoTheme()->icon(Theme::Icon::Export));
    m_ui->actionZoomIn->setIcon(mayoTheme()->icon(Theme::Icon::ZoomIn));
    m_ui->actionZoomOut->setIcon(mayoTheme()->icon(Theme::Icon::ZoomOut));
    m_ui->actionPreviousDoc->setIcon(mayoTheme()->icon(Theme::Icon::Back));
    m_ui->actionNextDoc->setIcon(mayoTheme()->icon(Theme::Icon::Next));
    m_ui->actionCloseDoc->setIcon(mayoTheme()->icon(Theme::Icon::Cross));
    m_ui->actionSaveImageView->setIcon(mayoTheme()->icon(Theme::Icon::Camera));
    m_ui->actionToggleLeftSidebar->setIcon(mayoTheme()->icon(Theme::Icon::LeftSidebar));
    m_ui->btn_CloseLeftSideBar->setIcon(mayoTheme()->icon(Theme::Icon::BackSquare));

    m_ui->actionToggleLeftSidebar->setChecked(m_ui->widget_Left->isVisible());
    m_ui->actionToggleFullscreen->setChecked(this->isFullScreen());
    m_ui->actionToggleOriginTrihedron->setChecked(false);

    mayoTheme()->setupHeaderComboBox(m_ui->combo_LeftContents);
    mayoTheme()->setupHeaderComboBox(m_ui->combo_GuiDocuments);
    QString labelMainHomeText = m_ui->label_MainHome->text();
    labelMainHomeText.replace(
                QRegularExpression("color:#[0-9a-fA-F]{6,6};"), // ex: color:#0000ff
                QString("color:%1;").arg(qApp->palette().color(QPalette::Link).name()));
    m_ui->label_MainHome->setText(labelMainHomeText);

    auto sigComboIndexChanged = QOverload<int>::of(&QComboBox::currentIndexChanged);
    // "File" actions
    QObject::connect(
                m_ui->actionNewDoc, &QAction::triggered,
                this, &MainWindow::newDocument);
    QObject::connect(
                m_ui->actionOpen, &QAction::triggered,
                this, &MainWindow::openDocuments);
    QObject::connect(
                m_ui->actionImport, &QAction::triggered,
                this, &MainWindow::importInCurrentDoc);
    QObject::connect(
                m_ui->actionExportSelectedItems, &QAction::triggered,
                this, &MainWindow::exportSelectedItems);
    QObject::connect(
                m_ui->actionCloseDoc, &QAction::triggered,
                this, &MainWindow::closeCurrentDocument);
    QObject::connect(
                m_ui->actionCloseAllDocuments, &QAction::triggered,
                this, &MainWindow::closeAllDocuments);
    QObject::connect(
                m_ui->actionCloseAllExcept, &QAction::triggered,
                this, &MainWindow::closeAllDocumentsExceptCurrent);
    QObject::connect(
                m_ui->actionQuit, &QAction::triggered,
                this, &MainWindow::quitApp);
    QObject::connect(
                m_ui->label_MainHome, &QLabel::linkActivated,
                this, &MainWindow::onHomePageLinkActivated);
    QObject::connect(
                m_ui->menu_File, &QMenu::aboutToShow,
                this, &MainWindow::createMenuRecentFiles);
    // "Display" actions
    QObject::connect(
                m_ui->actionToggleOriginTrihedron, &QAction::toggled,
                this, &MainWindow::toggleCurrentDocOriginTrihedron);
    QObject::connect(
                m_ui->actionZoomIn, &QAction::triggered,
                this, &MainWindow::zoomInCurrentDoc);
    QObject::connect(
                m_ui->actionZoomOut, &QAction::triggered,
                this, &MainWindow::zoomOutCurrentDoc);
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
                m_ui->actionToggleFullscreen, &QAction::toggled,
                this, &MainWindow::toggleFullscreen);
    QObject::connect(
                m_ui->actionToggleLeftSidebar, &QAction::toggled,
                this, &MainWindow::toggleLeftSidebar);
    QObject::connect(m_ui->actionPreviousDoc, &QAction::triggered, [=]{
        this->setCurrentDocumentIndex(this->currentDocumentIndex() - 1);
    });
    QObject::connect(m_ui->actionNextDoc, &QAction::triggered, [=]{
        this->setCurrentDocumentIndex(this->currentDocumentIndex() + 1);
    });
    QObject::connect(
                m_ui->combo_GuiDocuments, sigComboIndexChanged,
                this, &MainWindow::currentDocumentIndexChanged);
    QObject::connect(
                this, &MainWindow::currentDocumentIndexChanged,
                this, &MainWindow::onCurrentDocumentIndexChanged);
    QObject::connect(
                m_ui->widget_FileSystem, &WidgetFileSystem::locationActivated,
                this, &MainWindow::onWidgetFileSystemLocationActivated);
    // Left header bar of controls
    QObject::connect(
                m_ui->btn_CloseLeftSideBar, &QAbstractButton::clicked,
                this, &MainWindow::toggleLeftSidebar);
    // ...
    QObject::connect(
                m_ui->combo_LeftContents, sigComboIndexChanged,
                this, &MainWindow::onLeftContentsPageChanged);
    QObject::connect(
                GuiApplication::instance(), &GuiApplication::guiDocumentAdded,
                this, &MainWindow::onGuiDocumentAdded);
    QObject::connect(
                GuiApplication::instance()->selectionModel(),
                &ApplicationItemSelectionModel::changed,
                this,
                &MainWindow::onApplicationItemSelectionChanged);
    QObject::connect(
                m_ui->listView_OpenedDocuments, &QListView::clicked,
                [=](const QModelIndex& index) {
        this->setCurrentDocumentIndex(index.row());
    });
    QObject::connect(
                this, &MainWindow::operationFinished,
                this, &MainWindow::onOperationFinished);

    // Creation of annex objects
    {
        // Opened documents GUI
        auto listViewBtns =
                new qtgui::ItemViewButtons(m_ui->listView_OpenedDocuments, this);
        listViewBtns->addButton(
                    1, mayoTheme()->icon(Theme::Icon::Cross), m_ui->actionCloseDoc->toolTip());
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

    // BEWARE MainWindow::onGuiDocumentAdded() must be called before
    // MainWindow::onCurrentDocumentIndexChanged()
    auto guiDocModel = new GuiDocumentListModel(GuiApplication::instance());
    m_ui->combo_GuiDocuments->setModel(guiDocModel);
    m_ui->listView_OpenedDocuments->setModel(guiDocModel);

    // Finialize setup
    this->setAcceptDrops(true);
    m_ui->widget_LeftHeader->installEventFilter(this);
    m_ui->widget_ControlGuiDocuments->installEventFilter(this);
    m_ui->stack_GuiDocuments->installEventFilter(this);
    this->onLeftContentsPageChanged(m_ui->stack_LeftContents->currentIndex());
    this->updateControlsActivation();
    m_ui->widget_ApplicationTree->setReferenceItemTextTemplate(
                Options::instance()->referenceItemTextTemplate());
    m_ui->widget_MouseCoords->hide();

    this->onCurrentDocumentIndexChanged(-1);
}

MainWindow::~MainWindow()
{
    delete m_ui;
    Options::instance()->setRecentFiles(m_listRecentFile);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    auto funcSizeBtn = [](const QWidget* container, const QWidget* widgetHeightRef) {
        const int btnSideLen = widgetHeightRef->frameGeometry().height();
        const QList<QAbstractButton*> listBtn = container->findChildren<QAbstractButton*>();
        for (QAbstractButton* btn : listBtn)
            btn->setFixedSize(btnSideLen, btnSideLen);
    };
    const QEvent::Type eventType = event->type();
    if (watched == m_ui->widget_ControlGuiDocuments && eventType == QEvent::Show) {
        funcSizeBtn(m_ui->widget_ControlGuiDocuments, m_ui->combo_GuiDocuments);
        return true;
    }
    if (watched == m_ui->widget_LeftHeader && eventType == QEvent::Show) {
        funcSizeBtn(m_ui->widget_LeftHeader, m_ui->combo_LeftContents);
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

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QList<QUrl> listUrl = event->mimeData()->urls();
    QStringList listFilePath;
    for (const QUrl& url : listUrl) {
        if (url.isLocalFile())
            listFilePath.push_back(url.toLocalFile());
    }
    event->acceptProposedAction();
    this->openDocumentsFromList(listFilePath);
}

void MainWindow::newDocument()
{
    static unsigned docSequenceId = 0;
    auto doc = new Document;
    doc->setLabel(tr("Anonymous%1").arg(++docSequenceId));
    Application::instance()->addDocument(doc);
}

void MainWindow::openDocuments()
{
    const auto resFileNames = Internal::OpenFileNames::get(this);
    if (!resFileNames.listFilepath.isEmpty())
        this->openDocumentsFromList(resFileNames.listFilepath);
}

void MainWindow::importInCurrentDoc()
{
    auto widgetGuiDoc = this->currentWidgetGuiDocument();
    if (widgetGuiDoc != nullptr) {
        Document* doc = widgetGuiDoc->guiDocument()->document();
        const auto resFileNames = Internal::OpenFileNames::get(this);
        if (!resFileNames.listFilepath.isEmpty()) {
            const Application::PartFormat userFormat = resFileNames.selectedFormat;
            const bool hasUserFormat = userFormat != Application::PartFormat::Unknown;
            for (const QString& filepath : resFileNames.listFilepath) {
                const Application::PartFormat fileFormat =
                        hasUserFormat ? userFormat : Application::findPartFormat(filepath);
                if (fileFormat != Application::PartFormat::Unknown) {
                    this->runImportTask(doc, fileFormat, filepath);
                    Internal::prependRecentFile(&m_listRecentFile, filepath);
                }
                else {
                    Internal::msgBoxErrorFileFormat(this, filepath);
                }
            }
        }
    }
}

void MainWindow::runImportTask(
        Document* doc, Application::PartFormat format, const QString& filepath)
{
    auto task = qttask::Manager::globalInstance()->newTask<qttask::StdAsync>();
    task->setTaskTitle(QFileInfo(filepath).fileName());
    task->run([=]{
        QTime chrono;
        chrono.start();
        const Application::IoResult result =
                Application::instance()->importInDocument(
                    doc, format, filepath, &task->progress());
        QString msg;
        if (result) {
            msg = tr("Import time '%1': %2ms")
                    .arg(QFileInfo(filepath).fileName())
                    .arg(chrono.elapsed());
        } else {
            msg = tr("Failed to import part:\n    %1\nError: %2")
                    .arg(filepath, result.errorText());
        }
        emit operationFinished(result.valid(), msg);
    });
}

void MainWindow::runExportTask(
        const std::vector<DocumentItem*>& docItems,
        Application::PartFormat format,
        const Application::ExportOptions& opts,
        const QString& filepath)
{
    auto task = qttask::Manager::globalInstance()->newTask<qttask::StdAsync>();
    task->setTaskTitle(QFileInfo(filepath).fileName());
    task->run([=]{
        QTime chrono;
        chrono.start();
        const Application::IoResult result =
                Application::instance()->exportDocumentItems(
                    docItems, format, opts, filepath, &task->progress());
        QString msg;
        if (result) {
            msg = tr("Export time '%1': %2ms")
                    .arg(QFileInfo(filepath).fileName())
                    .arg(chrono.elapsed());
        } else {
            msg = tr("Failed to export part:\n    %1\nError: %2")
                    .arg(filepath).arg(result.errorText());
        }
        emit operationFinished(result.valid(), msg);
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
                GuiApplication::instance()->selectionModel()->selectedDocumentItems();
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

void MainWindow::toggleCurrentDocOriginTrihedron()
{
    WidgetGuiDocument* widget = this->currentWidgetGuiDocument();
    if (widget) {
        widget->guiDocument()->toggleOriginTrihedronVisibility();
        widget->guiDocument()->updateV3dViewer();
    }
}

void MainWindow::zoomInCurrentDoc()
{
    this->currentWidgetGuiDocument()->controller()->zoomIn();
}

void MainWindow::zoomOutCurrentDoc()
{
    this->currentWidgetGuiDocument()->controller()->zoomOut();
}

void MainWindow::editOptions()
{
    auto dlg = new DialogOptions(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::saveImageView()
{
    auto widgetGuiDoc = this->currentWidgetGuiDocument();
    auto dlg = new DialogSaveImageView(widgetGuiDoc->guiDocument()->v3dView());
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::inspectXde()
{
    const Span<const ApplicationItem> spanAppItem =
            GuiApplication::instance()->selectionModel()->selectedItems();
    const XdeDocumentItem* xdeDocItem = nullptr;
    for (const ApplicationItem& appItem : spanAppItem) {
        xdeDocItem = dynamic_cast<const XdeDocumentItem*>(appItem.documentItem());
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
}

void MainWindow::toggleLeftSidebar()
{
    const bool isVisible = m_ui->widget_Left->isVisible();
    m_ui->widget_Left->setVisible(!isVisible);
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

void MainWindow::onApplicationItemSelectionChanged()
{
    WidgetApplicationTree* uiAppTree = m_ui->widget_ApplicationTree;
    WidgetPropertiesEditor* uiProps = m_ui->widget_Properties;

    Span<const ApplicationItem> spanAppItem =
            GuiApplication::instance()->selectionModel()->selectedItems();
    if (spanAppItem.size() == 1) {
        const ApplicationItem& item = spanAppItem.at(0);
        if (item.isXdeAssemblyNode()) {
            const XdeAssemblyNode& xdeAsmNode = item.xdeAssemblyNode();
            const XdeDocumentItem* xdeItem = xdeAsmNode.ownerDocItem;
            const TDF_Label& xdeLabel = xdeAsmNode.label();
            using ShapePropsOption = XdeDocumentItem::ShapePropertiesOption;
            const ShapePropsOption opt =
                    uiAppTree->isMergeXdeReferredShapeOn() ?
                        ShapePropsOption::MergeReferred : ShapePropsOption::None;
            m_ptrXdeShapeProperties = xdeItem->shapeProperties(xdeLabel, opt);
            XdeShapePropertyOwner* shapeProps = m_ptrXdeShapeProperties.get();
            uiProps->editProperties(shapeProps);
            if (shapeProps) {
                QObject::connect(shapeProps, &XdeShapePropertyOwner::nameChanged, [=]{
                    uiAppTree->refreshItemText(item);
                });
                QObject::connect(shapeProps, &XdeShapePropertyOwner::referredNameChanged, [=]{
                    uiAppTree->refreshItemText(item);
                });
            }
        }
        else if (item.isDocumentItem()) {
            uiProps->editProperties(item.documentItem());
        }
        else if (item.isDocument()) {
            uiProps->editProperties(item.document());
        }

        if (QSettings().value(Internal::keyLinkWithDocumentSelector, false).toBool()) {
            const Document* doc = item.document();
            if (doc != nullptr) {
                const std::vector<Document*>& vecDoc = Application::instance()->documents();
                auto itFound = std::find(vecDoc.cbegin(), vecDoc.cend(), doc);
                auto index = itFound != vecDoc.cend() ? itFound - vecDoc.cbegin() : -1;
                if (index != -1)
                    this->setCurrentDocumentIndex(index);
            }
        }
    }
    else {
        // TODO
        uiProps->clear();
    }
    this->updateControlsActivation();
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
    if (Options::instance()->defaultShowOriginTrihedron()) {
        guiDoc->toggleOriginTrihedronVisibility();
        guiDoc->updateV3dViewer();
    }

    BaseV3dViewController* ctrl = widget->controller();
    QObject::connect(ctrl, &BaseV3dViewController::mouseMoved, [=](const QPoint& pos2d) {
        if (ctrl->isPanning() || ctrl->isRotating())
            return;
        auto selector = guiDoc->aisInteractiveContext()->MainSelector();
        selector->Pick(pos2d.x(), pos2d.y(), guiDoc->v3dView());
        const gp_Pnt pos3d =
                selector->NbPicked() > 0 ?
                    selector->PickedPoint(1) :
                    GpxUtils::V3dView_to3dPosition(guiDoc->v3dView(), pos2d.x(), pos2d.y());
        m_ui->label_ValuePosX->setText(QString::number(pos3d.X(), 'f', 3));
        m_ui->label_ValuePosY->setText(QString::number(pos3d.Y(), 'f', 3));
        m_ui->label_ValuePosZ->setText(QString::number(pos3d.Z(), 'f', 3));
    });

    m_ui->stack_GuiDocuments->addWidget(widget);
    this->updateControlsActivation();
    this->setCurrentDocumentIndex(Application::instance()->documentCount() - 1);
}

void MainWindow::onWidgetFileSystemLocationActivated(const QFileInfo &loc)
{
    this->openDocumentsFromList(QStringList(loc.absoluteFilePath()));
}

void MainWindow::onLeftContentsPageChanged(int pageId)
{
    m_ui->stack_LeftContents->setCurrentIndex(pageId);
    QWidget* placeHolder = this->recreateLeftHeaderPlaceHolder();
    if (m_ui->stack_LeftContents->currentWidget() == m_ui->page_ApplicationTree
        && placeHolder != nullptr)
    {
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

void MainWindow::onCurrentDocumentIndexChanged(int idx)
{
    m_ui->stack_GuiDocuments->setCurrentIndex(idx);
    QAbstractItemView* view = m_ui->listView_OpenedDocuments;
    view->setCurrentIndex(view->model()->index(idx, 0));

    this->updateControlsActivation();

    auto funcFilepathQuoted = [](const QString& filepath) {
        for (QChar c : filepath) {
            if (c.isSpace())
                return "\"" + filepath + "\"";
        }
        return filepath;
    };
    const Document* doc = Application::instance()->documentAt(idx);
    const QString textActionClose =
            doc != nullptr ?
                tr("Close %1").arg(funcFilepathQuoted(doc->label())) :
                tr("Close");
    const QString textActionCloseAllExcept =
            doc != nullptr ?
                tr("Close all except %1").arg(funcFilepathQuoted(doc->label())) :
                tr("Close all except current");
    const QString docFilePath =
            doc != nullptr ? doc->filePath() : QString();
    m_ui->actionCloseDoc->setText(textActionClose);
    m_ui->actionCloseAllExcept->setText(textActionCloseAllExcept);
    m_ui->widget_FileSystem->setLocation(docFilePath);

    if (this->currentWidgetGuiDocument() != nullptr) {
        QSignalBlocker sigBlock(m_ui->actionToggleOriginTrihedron); Q_UNUSED(sigBlock);
        const GuiDocument* guiDoc = this->currentWidgetGuiDocument()->guiDocument();
        m_ui->actionToggleOriginTrihedron->setChecked(guiDoc->isOriginTrihedronVisible());
    }
    else {
        m_ui->actionToggleOriginTrihedron->setChecked(false);
    }
 }

void MainWindow::closeCurrentDocument()
{
    this->closeDocument(this->currentDocumentIndex());
}

void MainWindow::closeDocument(WidgetGuiDocument *widget)
{
    if (widget != nullptr) {
        Document* doc = widget->guiDocument()->document();
        m_ui->stack_GuiDocuments->removeWidget(widget);
        widget->deleteLater();
        Application::instance()->eraseDocument(doc);
        this->updateControlsActivation();
    }
}

void MainWindow::closeDocument(int docIndex)
{
    if (0 <= docIndex && docIndex < m_ui->stack_GuiDocuments->count())
        this->closeDocument(this->widgetGuiDocument(docIndex));
}

void MainWindow::closeAllDocumentsExceptCurrent()
{
    WidgetGuiDocument* current = this->currentWidgetGuiDocument();
    std::vector<WidgetGuiDocument*> vecWidget;
    for (int i = 0; i < m_ui->stack_GuiDocuments->count(); ++i)
        vecWidget.push_back(this->widgetGuiDocument(i));
    for (WidgetGuiDocument* widget : vecWidget) {
        if (widget != current)
            this->closeDocument(widget);
    }
}

void MainWindow::closeAllDocuments()
{
    while (m_ui->stack_GuiDocuments->count() > 0)
        this->closeCurrentDocument();
}

void MainWindow::openDocumentsFromList(const QStringList& listFilePath)
{
    auto app = Application::instance();
    const Application::ArrayDocument& vecDocument = app->documents();
    for (const QString& filePath : listFilePath) {
        const QFileInfo loc(filePath);
        auto itDocFound = app->findDocumentByLocation(loc);
        if (itDocFound == vecDocument.cend()) {
            const QString locAbsoluteFilePath =
                    QDir::toNativeSeparators(loc.absoluteFilePath());
            const Application::PartFormat fileFormat =
                    Application::findPartFormat(locAbsoluteFilePath);
            if (fileFormat != Application::PartFormat::Unknown) {
                auto doc = new Document;
                doc->setLabel(loc.fileName());
                doc->setFilePath(locAbsoluteFilePath);
                app->addDocument(doc);
                this->runImportTask(doc, fileFormat, locAbsoluteFilePath);
                Internal::prependRecentFile(&m_listRecentFile, locAbsoluteFilePath);
            }
            else {
                Internal::msgBoxErrorFileFormat(this, locAbsoluteFilePath);
            }
        }
        else {
            if (listFilePath.size() == 1)
                this->setCurrentDocumentIndex(itDocFound - vecDocument.cbegin());
        }
    }
}

void MainWindow::updateControlsActivation()
{
    const QWidget* currMainPage = m_ui->stack_Main->currentWidget();
    const int appDocumentsCount = Application::instance()->documentCount();
    const bool appDocumentsEmpty = appDocumentsCount == 0;
    QWidget* newMainPage =
            appDocumentsEmpty ? m_ui->page_MainHome : m_ui->page_MainControl;
    if (currMainPage != newMainPage)
        m_ui->stack_Main->setCurrentWidget(newMainPage);
    m_ui->actionImport->setEnabled(!appDocumentsEmpty);
    m_ui->actionToggleOriginTrihedron->setEnabled(!appDocumentsEmpty);
    m_ui->actionZoomIn->setEnabled(!appDocumentsEmpty);
    m_ui->actionZoomOut->setEnabled(!appDocumentsEmpty);
    m_ui->actionSaveImageView->setEnabled(!appDocumentsEmpty);
    m_ui->actionCloseDoc->setEnabled(!appDocumentsEmpty);
    m_ui->actionCloseAllDocuments->setEnabled(!appDocumentsEmpty);
    m_ui->actionCloseAllExcept->setEnabled(!appDocumentsEmpty);
    const int currentDocIndex = this->currentDocumentIndex();
    m_ui->actionPreviousDoc->setEnabled(
                !appDocumentsEmpty && currentDocIndex > 0);
    m_ui->actionNextDoc->setEnabled(
                !appDocumentsEmpty && currentDocIndex < appDocumentsCount - 1);
    m_ui->actionExportSelectedItems->setEnabled(!appDocumentsEmpty);
    m_ui->actionToggleLeftSidebar->setEnabled(newMainPage != m_ui->page_MainHome);
    m_ui->combo_GuiDocuments->setEnabled(!appDocumentsEmpty);

    Span<const ApplicationItem> spanSelectedAppItem =
            GuiApplication::instance()->selectionModel()->selectedItems();
    const ApplicationItem firstAppItem =
            !spanSelectedAppItem.empty() ?
                spanSelectedAppItem.at(0) : ApplicationItem();
    m_ui->actionInspectXDE->setEnabled(
                spanSelectedAppItem.size() == 1
                && sameType<XdeDocumentItem>(firstAppItem.documentItem()));
}

int MainWindow::currentDocumentIndex() const
{
    return m_ui->combo_GuiDocuments->currentIndex();
}

void MainWindow::setCurrentDocumentIndex(int idx)
{
    m_ui->combo_GuiDocuments->setCurrentIndex(idx);
}

WidgetGuiDocument* MainWindow::widgetGuiDocument(int idx) const
{
    return qobject_cast<WidgetGuiDocument*>(
                m_ui->stack_GuiDocuments->widget(idx));
}

WidgetGuiDocument *MainWindow::currentWidgetGuiDocument() const
{
    return this->widgetGuiDocument(this->currentDocumentIndex());
}

QWidget* MainWindow::findLeftHeaderPlaceHolder() const
{
    return m_ui->widget_LeftHeader->findChild<QWidget*>(
        "LeftHeaderPlaceHolder", Qt::FindDirectChildrenOnly);
}

QWidget* MainWindow::recreateLeftHeaderPlaceHolder()
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

QMenu* MainWindow::createMenuModelTreeSettings()
{
    auto menu = new QMenu(this->findLeftHeaderPlaceHolder());
    menu->setToolTipsVisible(true);

    {   // Link with document selector
        const bool isLinked =
                QSettings().value(Internal::keyLinkWithDocumentSelector, false)
                .toBool();
        QAction* action = menu->addAction(tr("Link With Document Selector"));
        action->setCheckable(true);
        action->setChecked(isLinked);
        QObject::connect(action, &QAction::triggered, [=](bool on) {
            QSettings().setValue(Internal::keyLinkWithDocumentSelector, on);
        });
    }

    menu->addSeparator();
    {   // Reference item text mode
        auto group = new QActionGroup(menu);
        group->setExclusive(true);
        auto actionOnlyInstance =
                new QAction(tr("Show only name of reference entities"), menu);
        auto actionOnlyReferred =
                new QAction(tr("Show only name of referred entities"), menu);
        auto actionInstanceAndReferred =
                new QAction(tr("Show name of both reference and referred entities"), menu);

        Options* opts = Options::instance();
        using TextMode = Options::ReferenceItemTextMode;
        using MenuData = std::tuple<QAction*, TextMode>;
        const std::vector<MenuData> arrayMenuData = {
            { actionOnlyInstance, TextMode::ReferenceOnly },
            { actionOnlyReferred, TextMode::ReferredOnly },
            { actionInstanceAndReferred, TextMode::ReferenceAndReferred }
        };
        for (const MenuData& menuData : arrayMenuData) {
            QAction* action = std::get<0>(menuData);
            action->setCheckable(true);
            group->addAction(action);
            menu->addAction(action);
            if (std::get<1>(menuData) == opts->referenceItemTextMode())
                action->setChecked(true);
        }

        QObject::connect(group, &QActionGroup::triggered, [=](QAction* action){
            for (const MenuData& menuData : arrayMenuData) {
                if (std::get<0>(menuData) == action) {
                    const TextMode textMode = std::get<1>(menuData);
                    opts->setReferenceItemTextMode(textMode);
                    m_ui->widget_ApplicationTree->setReferenceItemTextTemplate(
                                Options::toReferenceItemTextTemplate(textMode));
                }
            }
        });
    }

    return menu;
}

QMenu* MainWindow::createMenuRecentFiles()
{
    QMenu* menu = m_ui->actionRecentFiles->menu();
    if (menu == nullptr)
        menu = new QMenu(this);
    menu->clear();
    int idFile = 0;
    for (const QString& file : m_listRecentFile) {
        const QString entryRecentFile = tr("%1 | %2").arg(++idFile).arg(file);
        menu->addAction(entryRecentFile, [=]{
            this->openDocumentsFromList(QStringList(file));
        });
    }
    if (!m_listRecentFile.empty()) {
        menu->addSeparator();
        menu->addAction(tr("Clear menu"), [=]{
            menu->clear();
            m_listRecentFile.clear();
        });
    }
    m_ui->actionRecentFiles->setMenu(menu);
    return menu;
}

} // namespace Mayo
