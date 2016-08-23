#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "document.h"
#include "ui_document.h"
#include "task_manager_dialog.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/runner_qthreadpool.h"

#include <QtCore/QSettings>
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
                m_ui->tab_Documents, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);

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
    auto uiDoc = new UiDocument(doc);
    QObject::connect(uiDoc, &QObject::destroyed, doc, &QObject::deleteLater);
    m_ui->tab_Documents->addTab(uiDoc, tr("New-%1").arg(++newDocId));
    m_ui->tab_Documents->setCurrentWidget(uiDoc);
    this->updateControlsActivation();
}

void MainWindow::openPartInNewDoc()
{
}

void MainWindow::importPartInCurrentDoc()
{
    auto uiDoc = qobject_cast<UiDocument*>(m_ui->tab_Documents->currentWidget());
    if (uiDoc != nullptr) {
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
            Document* doc = uiDoc->document();
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
                const bool importOk =
                        doc->import(format, filepath, &task->progress());
                if (!importOk) {
                    qtgui::QWidgetUtils::asyncMsgBoxCritical(
                                this,
                                tr("Error"),
                                tr("Failed to import part:\n    '%1'")
                                .arg(filepath));
                }
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
