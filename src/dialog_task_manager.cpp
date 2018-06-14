/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_task_manager.h"

#include "ui_dialog_task_manager.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/progress.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QToolButton>

namespace Mayo {

namespace Internal {

static const char TaskWidget_taskIdProp[] = "Mayo::TaskId";

class TaskWidget : public QWidget
{
public:
    TaskWidget(QWidget* parent = nullptr);

    QLabel* m_label = nullptr;
    QProgressBar* m_progress = nullptr;
    QToolButton* m_interruptBtn = nullptr;

    void createUnboundedProgressTimer();
    void stopUnboundedProgressTimer();
    bool hasUnboundedProgressTimer() const;

private:
    void onUnboundedProgressTimeout();

    QTimer* m_unboundedProgressTimer = nullptr;
    int m_unboundedProgressValue = 0;
};

TaskWidget::TaskWidget(QWidget *parent)
    : QWidget(parent),
      m_label(new QLabel(this)),
      m_progress(new QProgressBar(this)),
      m_interruptBtn(new QToolButton(this))
{
    QFont labelFont = m_label->font();
    labelFont.setBold(true);
    m_label->setFont(labelFont);

    m_progress->setRange(0, 100);
    m_progress->setValue(0);

    m_interruptBtn->setIcon(QPixmap(":/images/no.png"));
    m_interruptBtn->setAutoRaise(true);

    auto progressLayout = new QHBoxLayout;
    progressLayout->addWidget(m_progress);
    progressLayout->addWidget(m_interruptBtn);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_label);
    mainLayout->addLayout(progressLayout);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
}

void TaskWidget::createUnboundedProgressTimer()
{
    if (!this->hasUnboundedProgressTimer()) {
        m_unboundedProgressTimer = new QTimer(this);
        QObject::connect(
                    m_unboundedProgressTimer, &QTimer::timeout,
                    this, &TaskWidget::onUnboundedProgressTimeout);
        m_unboundedProgressTimer->start(500);
    }
}

void TaskWidget::stopUnboundedProgressTimer()
{
    if (m_unboundedProgressTimer != nullptr)
        m_unboundedProgressTimer->stop();
}

bool TaskWidget::hasUnboundedProgressTimer() const
{
    return m_unboundedProgressTimer != nullptr;
}

void TaskWidget::onUnboundedProgressTimeout()
{
    m_unboundedProgressValue += 5;
    m_progress->setValue(m_unboundedProgressValue % 100);
}

} // namespace Internal

DialogTaskManager::DialogTaskManager(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogTaskManager)
{
    this->setWindowFlags(
                Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    m_ui->setupUi(this);

    auto taskMgr = qttask::Manager::globalInstance();
    QObject::connect(
                taskMgr, &qttask::Manager::started,
                this, &DialogTaskManager::onTaskStarted);
    QObject::connect(
                taskMgr, &qttask::Manager::ended,
                this, &DialogTaskManager::onTaskEnded);
    QObject::connect(
                taskMgr, &qttask::Manager::progress,
                this, &DialogTaskManager::onTaskProgress);
    QObject::connect(
                taskMgr, &qttask::Manager::progressStep,
                this, &DialogTaskManager::onTaskProgressStep);
    this->setWindowModality(Qt::WindowModal);
}

DialogTaskManager::~DialogTaskManager()
{
    delete m_ui;
}

bool DialogTaskManager::isRunning() const
{
    return m_isRunning;
}

void DialogTaskManager::execWithTask(quint64 taskId)
{
    m_isRunning = true;
    this->onTaskStarted(taskId, QString());
    this->exec();
}

void DialogTaskManager::onTaskStarted(quint64 taskId, const QString& title)
{
    if (!m_isRunning)
        this->show();

    auto widget = new Internal::TaskWidget(m_ui->scrollAreaContents);
    widget->m_interruptBtn->setProperty(Internal::TaskWidget_taskIdProp, taskId);
    QObject::connect(
                widget->m_interruptBtn, &QToolButton::clicked,
                this, &DialogTaskManager::interruptTask);
    m_ui->contentsLayout->insertWidget(0, widget);
    m_taskIdToWidget.insert(taskId, widget);

    ++m_taskCount;

    if (!title.isEmpty())
        this->onTaskProgressStep(taskId, title);
}

void DialogTaskManager::onTaskEnded(quint64 taskId)
{
    Internal::TaskWidget* widget = this->taskWidget(taskId);
    if (widget != nullptr) {
        if (widget->hasUnboundedProgressTimer())
            widget->stopUnboundedProgressTimer();
        m_ui->contentsLayout->removeWidget(widget);
        delete widget;
        m_taskIdToWidget.remove(taskId);
    }

    --m_taskCount;

    if (m_taskCount == 0) {
        m_isRunning = false;
        this->accept();
    }
}

void DialogTaskManager::onTaskProgress(quint64 taskId, int percent)
{
    Internal::TaskWidget* widget = this->taskWidget(taskId);
    if (widget != nullptr) {
        if (percent >= 0) {
            widget->m_progress->setValue(percent);
        }
        else {
            widget->createUnboundedProgressTimer();
            widget->m_progress->setTextVisible(false);
        }
    }
}

void DialogTaskManager::onTaskProgressStep(quint64 taskId, const QString& name)
{
    Internal::TaskWidget* widget = this->taskWidget(taskId);
    if (widget != nullptr)
        widget->m_label->setText(name);
}

void DialogTaskManager::interruptTask()
{
    auto interruptBtn = qobject_cast<QToolButton*>(this->sender());
    if (interruptBtn != nullptr
            && interruptBtn->dynamicPropertyNames().contains(Internal::TaskWidget_taskIdProp))
    {
        const quint64 taskId =
                interruptBtn->property(Internal::TaskWidget_taskIdProp).toULongLong();
        qttask::Manager::globalInstance()->requestAbort(taskId);
    }
}

Internal::TaskWidget *DialogTaskManager::taskWidget(quint64 taskId)
{
    auto it = m_taskIdToWidget.find(taskId);
    return it != m_taskIdToWidget.end() ? it.value() : nullptr;
}

} // namespace RondPointApp
