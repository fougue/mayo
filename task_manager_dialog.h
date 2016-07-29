#pragma once

#include <QtCore/QHash>
#include <QtWidgets/QDialog>

namespace Mayo {

namespace Internal { class TaskWidget; }

class TaskManagerDialog : public QDialog
{
    Q_OBJECT

public:
    TaskManagerDialog(QWidget *parent = nullptr);
    ~TaskManagerDialog();

    bool isRunning() const;
    void execWithTask(quint64 taskId);

private:
    void onTaskStarted(quint64 taskId, const QString& title);
    void onTaskEnded(quint64 taskId);
    void onTaskProgress(quint64 taskId, int percent);
    void onTaskProgressStep(quint64 taskId, const QString& name);
    void interruptTask();

    Internal::TaskWidget* taskWidget(quint64 taskId);

    class Ui_TaskManagerDialog* m_ui = nullptr;
    QHash<quint64, Internal::TaskWidget*> m_taskIdToWidget;
    bool m_isRunning = false;
    unsigned m_taskCount = 0;
};

} // namespace Mayo
