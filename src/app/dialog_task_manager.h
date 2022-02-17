/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/task_common.h"

#include <QtWidgets/QDialog>
#include <unordered_map>

namespace Mayo {

class TaskManager;

class DialogTaskManager : public QDialog {
    Q_OBJECT
public:
    DialogTaskManager(TaskManager* taskMgr, QWidget* parent = nullptr);
    ~DialogTaskManager();

private:
    class TaskWidget;
    TaskWidget* taskWidget(TaskId taskId);

    void onTaskStarted(TaskId taskId);
    void onTaskEnded(TaskId taskId);
    void onTaskProgress(TaskId taskId, int percent);
    void onTaskProgressStep(TaskId taskId, std::string_view name);
    void interruptTask();

    class Ui_DialogTaskManager* m_ui = nullptr;
    TaskManager* m_taskMgr = nullptr;
    std::unordered_map<TaskId, TaskWidget*> m_taskIdToWidget;
    bool m_isRunning = false;
    unsigned m_taskCount = 0;
};

} // namespace Mayo
