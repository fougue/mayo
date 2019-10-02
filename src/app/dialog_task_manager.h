/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QHash>
#include <QtWidgets/QDialog>

namespace Mayo {

class DialogTaskManager : public QDialog {
    Q_OBJECT
public:
    DialogTaskManager(QWidget* parent = nullptr);
    ~DialogTaskManager();

private:
    class TaskWidget;
    TaskWidget* taskWidget(quint64 taskId);

    void onTaskStarted(quint64 taskId, const QString& title);
    void onTaskEnded(quint64 taskId);
    void onTaskProgress(quint64 taskId, int percent);
    void onTaskProgressStep(quint64 taskId, const QString& name);
    void interruptTask();

    class Ui_DialogTaskManager* m_ui = nullptr;
    QHash<quint64, TaskWidget*> m_taskIdToWidget;
    bool m_isRunning = false;
    unsigned m_taskCount = 0;
};

} // namespace Mayo
