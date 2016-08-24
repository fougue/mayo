#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "about_dialog.h"
#include "document.h"
#include "document_view.h"
#include "message_indicator.h"
#include "task_manager_dialog.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/runner_qthreadpool.h"

#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>

namespace Mayo {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_ui(new Ui_MainWindow)
{
    m_ui->setupUi(this);

    new TaskManagerDialog(this);

    QObject::connect(
                m_ui->actionNewDoc, &QAction::triggered,
                this, &MainWindow::newDoc);
    QObject::connect(
                m_ui->actionOpen, &QAction::triggered,
                this, &MainWindow::openPartInNewDoc);
    QObject::connect(
                m_ui->actionImportPart, &QAction::triggered,
                this, &MainWindow::importPartInCurrentDoc);
    QObject::connect(
                m_ui->actionQuit, &QAction::triggered,
                this, &MainWindow::quitApp);
    QObject::connect(
                m_ui->actionReportBug, &QAction::triggered,
                this, &MainWindow::reportbug);
    QObject::connect(
                m_ui->actionAboutMayo, &QAction::triggered,
                this, &MainWindow::aboutMayo);
    QObject::connect(
                m_ui->tab_Documents, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);
    QObject::connect(
                this, &MainWindow::importPartFinished,
                this, &MainWindow::onImportPartFinished);

    this->updateControlsActivation();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::newDoc()
{
    static unsigned newDocId = 0;
    auto doc = new Document(this);
    auto docView = new DocumentView(doc);
    QObject::connect(docView, &QObject::destroyed, doc, &QObject::deleteLater);
    m_ui->tab_Documents->addTab(docView, tr("New-%1").arg(++newDocId));
    m_ui->tab_Documents->setCurrentWidget(docView);
    this->updateControlsActivation();
}

void MainWindow::openPartInNewDoc()
{
    qtgui::QWidgetUtils::asyncMsgBoxCritical(
                this, tr("Error"), tr("Not yet implemented"));
}

void MainWindow::importPartInCurrentDoc()
{
    auto docView =
            qobject_cast<DocumentView*>(m_ui->tab_Documents->currentWidget());
    if (docView != nullptr) {
        QSettings settings;
        static const QLatin1String keyLastOpenDir("GUI/MainWindow_lastOpenDir");
        const QString lastOpenDir =
                settings.value(keyLastOpenDir, QString()).toString();
        const QString filters =
                Document::partFormatFilters().join(QLatin1String(";;"));
        QString selectedFilter;
        const QString filepath =
                QFileDialog::getOpenFileName(
                    this,
                    tr("Select Part File"),
                    lastOpenDir,
                    filters,
                    &selectedFilter);
        if (!filepath.isEmpty()) {
            Document* doc = docView->document();
            const auto itFormatFound =
                    std::find_if(Document::partFormats().cbegin(),
                                 Document::partFormats().cend(),
                                 [=](Document::PartFormat format) {
                return selectedFilter == Document::partFormatFilter(format);
            } );
            const Document::PartFormat format =
                    itFormatFound != Document::partFormats().cend() ?
                        *itFormatFound :
                        Document::PartFormat::Unknown;

            qttask::BaseRunner* task =
                    qttask::Manager::globalInstance()->newTask<QThreadPool>();
            task->run([=]{
                QTime chrono;
                chrono.start();
                const bool importOk =
                        doc->import(format, filepath, &task->progress());
                const QString msg =
                        importOk ?
                            tr("Import time: %1ms").arg(chrono.elapsed()) :
                            tr("Failed to import part:\n    '%1'").arg(filepath);
                emit importPartFinished(importOk, filepath, msg);
            });

            settings.setValue(
                        keyLastOpenDir, QFileInfo(filepath).canonicalPath());
        }
    }
}

void MainWindow::quitApp()
{
    QApplication::quit();
}

void MainWindow::aboutMayo()
{
    auto dlg = new AboutDialog(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::reportbug()
{
    QDesktopServices::openUrl(
                QUrl(QStringLiteral("https://github.com/fougue/mayo/issues")));
}

void MainWindow::onImportPartFinished(
        bool ok, const QString &/*filepath*/, const QString &msg)
{
    if (ok)
        MessageIndicator::showMessage(msg, this);
    else
        qtgui::QWidgetUtils::asyncMsgBoxCritical(this, tr("Error"), msg);
}

void MainWindow::onTabCloseRequested(int tabIndex)
{
    QWidget* tab = m_ui->tab_Documents->widget(tabIndex);
    m_ui->tab_Documents->removeTab(tabIndex);
    tab->deleteLater();
    this->updateControlsActivation();
}

void MainWindow::updateControlsActivation()
{
    m_ui->actionImportPart->setEnabled(m_ui->tab_Documents->count() > 0);
}

} // namespace Mayo
