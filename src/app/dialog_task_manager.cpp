/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_task_manager.h"

#include "../base/task_manager.h"
#include "../qtcommon/qstring_conv.h"
#include "ui_dialog_task_manager.h"
#include "app_module.h"
#include "qstring_utils.h"
#include "theme.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QToolButton>

namespace Mayo {

// --
// -- DialogTaskManager::TaskWidget
// --

class DialogTaskManager::TaskWidget : public QWidget {
public:
    TaskWidget(QWidget* parent = nullptr);

    QLabel* m_label = nullptr;
    QProgressBar* m_progress = nullptr;
    QToolButton* m_interruptBtn = nullptr;

    void createUnboundedProgressTimer();
    void stopUnboundedProgressTimer();
    bool hasUnboundedProgressTimer() const;

    static const char TaskIdProp[];

private:
    void onUnboundedProgressTimeout();

    QTimer* m_unboundedProgressTimer = nullptr;
    int m_unboundedProgressValue = 0;
};

const char DialogTaskManager::TaskWidget::TaskIdProp[] = "Mayo::TaskId";

DialogTaskManager::TaskWidget::TaskWidget(QWidget* parent)
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
    m_interruptBtn->setIcon(mayoTheme()->icon(Theme::Icon::Stop));
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

void DialogTaskManager::TaskWidget::createUnboundedProgressTimer()
{
    if (!this->hasUnboundedProgressTimer()) {
        m_unboundedProgressTimer = new QTimer(this);
        QObject::connect(
                    m_unboundedProgressTimer, &QTimer::timeout,
                    this, &TaskWidget::onUnboundedProgressTimeout);
        m_unboundedProgressTimer->start(500);
    }
}

void DialogTaskManager::TaskWidget::stopUnboundedProgressTimer()
{
    if (m_unboundedProgressTimer)
        m_unboundedProgressTimer->stop();
}

bool DialogTaskManager::TaskWidget::hasUnboundedProgressTimer() const
{
    return m_unboundedProgressTimer != nullptr;
}

void DialogTaskManager::TaskWidget::onUnboundedProgressTimeout()
{
    m_unboundedProgressValue += 5;
    m_progress->setValue(m_unboundedProgressValue % 100);
}

// --
// -- DialogTaskManager
// --

DialogTaskManager::DialogTaskManager(TaskManager* taskMgr, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogTaskManager),
      m_taskMgr(taskMgr)
{
    m_ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    this->setWindowModality(Qt::WindowModal);

    taskMgr->signalStarted.connectSlot(&DialogTaskManager::onTaskStarted, this);
    taskMgr->signalEnded.connectSlot(&DialogTaskManager::onTaskEnded, this);
    taskMgr->signalProgressChanged.connectSlot(&DialogTaskManager::onTaskProgress, this);
    taskMgr->signalProgressStep.connectSlot(&DialogTaskManager::onTaskProgressStep, this);
}

DialogTaskManager::~DialogTaskManager()
{
    delete m_ui;
}

void DialogTaskManager::onTaskStarted(TaskId taskId)
{
    if (!m_isRunning)
        this->show();

    auto widget = new TaskWidget(m_ui->scrollAreaContents);
    widget->m_interruptBtn->setProperty(TaskWidget::TaskIdProp, quint64(taskId));
    QObject::connect(
        widget->m_interruptBtn, &QToolButton::clicked,
        this, &DialogTaskManager::interruptTask
    );
    m_ui->contentsLayout->insertWidget(0, widget);
    m_taskIdToWidget.insert({ taskId, widget });
    ++m_taskCount;
    this->onTaskProgressStep(taskId, {});
}

void DialogTaskManager::onTaskEnded(TaskId taskId)
{
    TaskWidget* widget = this->taskWidget(taskId);
    if (widget) {
        if (widget->hasUnboundedProgressTimer())
            widget->stopUnboundedProgressTimer();

        m_ui->contentsLayout->removeWidget(widget);
        delete widget;
        m_taskIdToWidget.erase(taskId);
    }

    --m_taskCount;
    if (m_taskCount == 0) {
        m_isRunning = false;
        this->accept();
    }
}

void DialogTaskManager::onTaskProgress(TaskId taskId, int percent)
{
    TaskWidget* widget = this->taskWidget(taskId);
    if (widget) {
        if (percent >= 0) {
            widget->m_progress->setValue(percent);
        }
        else {
            widget->createUnboundedProgressTimer();
            widget->m_progress->setTextVisible(false);
        }
    }
}

void DialogTaskManager::onTaskProgressStep(TaskId taskId, std::string_view name)
{
    TaskWidget* widget = this->taskWidget(taskId);
    if (!widget)
        return;

    const QString taskTitle = to_QString(m_taskMgr->title(taskId));
    QString text = taskTitle;
    if (!name.empty()) {
        const QLocale& locale = AppModule::get()->qtLocale();
        if (!text.isEmpty())
            QStringUtils::append(&text, tr(" / "), locale);

        QStringUtils::append(&text, to_QString(name), locale);
    }

    widget->m_label->setText(text);
}

void DialogTaskManager::interruptTask()
{
    auto interruptBtn = qobject_cast<QToolButton*>(this->sender());
    if (interruptBtn && interruptBtn->dynamicPropertyNames().contains(TaskWidget::TaskIdProp)) {
        const quint64 taskId = interruptBtn->property(TaskWidget::TaskIdProp).toULongLong();
        m_taskMgr->requestAbort(taskId);
    }
}

DialogTaskManager::TaskWidget* DialogTaskManager::taskWidget(TaskId taskId)
{
    auto it = m_taskIdToWidget.find(taskId);
    return it != m_taskIdToWidget.end() ? it->second : nullptr;
}

} // namespace Mayo
