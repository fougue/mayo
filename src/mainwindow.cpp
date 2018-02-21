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
#include "xde_document_item.h"
#include "dialog_about.h"
#include "dialog_export_options.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "dialog_task_manager.h"
#include "document.h"
#include "gui_application.h"
#include "gui_document.h"
#include "widget_gui_document_view3d.h"
#include "widget_message_indicator.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/runner_qthreadpool.h"

#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>

#include "dialog_inspect_xde.h"
#include <STEPCAFControl_Reader.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>

namespace Mayo {

namespace Internal {

static const QLatin1String keyLastOpenDir("GUI/MainWindow_lastOpenDir");
static const QLatin1String keyLastSelectedFilter("GUI/MainWindow_lastSelectedFilter");

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

} // namespace Internal


MainWindow::MainWindow(GuiApplication *guiApp, QWidget *parent)
    : QMainWindow(parent),
      m_guiApp(guiApp),
      m_ui(new Ui_MainWindow)
{
    m_ui->setupUi(this);
    m_ui->widget_DocumentProps->setGuiApplication(guiApp);
    m_ui->widget_DocumentProps->editDocumentItems(std::vector<DocumentItem*>());

    new DialogTaskManager(this);

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
                m_ui->actionSaveImageView, &QAction::triggered,
                this, &MainWindow::saveImageView);
    QObject::connect(
                m_ui->actionInspectXDE, &QAction::triggered,
                this, &MainWindow::inspectXde);
    QObject::connect(
                m_ui->actionOptions, &QAction::triggered,
                this, &MainWindow::editOptions);
    QObject::connect(
                m_ui->actionReportBug, &QAction::triggered,
                this, &MainWindow::reportbug);
    QObject::connect(
                m_ui->actionAboutMayo, &QAction::triggered,
                this, &MainWindow::aboutMayo);
    QObject::connect(
                m_ui->tab_GuiDocuments, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);
    QObject::connect(
                this, &MainWindow::operationFinished,
                this, &MainWindow::onOperationFinished);

    QObject::connect(
                guiApp, &GuiApplication::guiDocumentAdded,
                this, &MainWindow::onGuiDocumentAdded);
    QObject::connect(
                m_ui->widget_ApplicationTree,
                &WidgetApplicationTree::selectionChanged,
                this,
                &MainWindow::onApplicationTreeWidgetSelectionChanged);

    this->updateControlsActivation();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::newDoc()
{
    Application::instance()->addDocument();
}

void MainWindow::openPartInNewDoc()
{
    this->foreachOpenFileName(
                [=](Application::PartFormat format, QString filepath) {
        Document* doc = Application::instance()->addDocument(
                    QFileInfo(filepath).fileName());
        this->runImportTask(doc, format, filepath);
    });
}

void MainWindow::importInCurrentDoc()
{
    auto docView3d =
            qobject_cast<WidgetGuiDocumentView3d*>(
                m_ui->tab_GuiDocuments->currentWidget());
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
    qttask::BaseRunner* task =
            qttask::Manager::globalInstance()->newTask<QThread>();
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
                    .arg(filepath).arg(result.errorText);
        }
        emit operationFinished(result.ok, msg);
    });
}

void MainWindow::exportSelectedItems()
{
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
            QObject::connect(
                        dlg, &QDialog::accepted,
                        [=]{
                this->runExportTask(
                            vecDocItem, format, dlg->currentExportOptions(), filepath);
                Internal::ImportExportSettings::save(lastSettings);
            });
            qtgui::QWidgetUtils::asyncDialogExec(dlg);
#else
            this->runExportTask(
                        vecDocItem, format, Application::ExportOptions(), filepath);
            Internal::ImportExportSettings::save(lastSettings);
#endif
        }
        else {
            this->runExportTask(
                        vecDocItem, format, Application::ExportOptions(), filepath);
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
    auto docView3d =
            qobject_cast<const WidgetGuiDocumentView3d*>(
                m_ui->tab_GuiDocuments->currentWidget());
    auto dlg = new DialogSaveImageView(docView3d->widgetOccView());
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

void MainWindow::onGuiDocumentAdded(GuiDocument *guiDoc)
{
    m_ui->tab_GuiDocuments->addTab(
                guiDoc->widgetView3d(), guiDoc->document()->label());
    m_ui->tab_GuiDocuments->setCurrentWidget(guiDoc->widgetView3d());
    this->updateControlsActivation();
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

void MainWindow::onTabCloseRequested(int tabIndex)
{
    QWidget* tab = m_ui->tab_GuiDocuments->widget(tabIndex);
    auto docView3d = qobject_cast<WidgetGuiDocumentView3d*>(tab);
    Application::instance()->eraseDocument(docView3d->guiDocument()->document());
    m_ui->tab_GuiDocuments->removeTab(tabIndex);
    this->updateControlsActivation();
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

void MainWindow::runExportTask(
        const std::vector<DocumentItem*>& docItems,
        Application::PartFormat format,
        const Application::ExportOptions& opts,
        const QString& filepath)
{
    qttask::BaseRunner* task =
            qttask::Manager::globalInstance()->newTask<QThread>();
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

void MainWindow::updateControlsActivation()
{
    const bool appDocumentsEmpty = Application::instance()->documents().empty();
    m_ui->actionImport->setEnabled(!appDocumentsEmpty);
    m_ui->actionSaveImageView->setEnabled(!appDocumentsEmpty);
}

} // namespace Mayo
