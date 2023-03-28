/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

// Module : GUI

#include "../../base/task_common.h"

#include <unordered_map>
#include <QtCore/QObject>
class QWindow;
class QWinTaskbarButton;

namespace Mayo {

class TaskManager;

class WinTaskbarGlobalProgress : public QObject {
    Q_OBJECT
public:
    WinTaskbarGlobalProgress(TaskManager* taskMgr, QObject* parent = nullptr);

    void setWindow(QWindow* window);

private:
    void onTaskProgress(TaskId taskId, int percent);
    void onTaskEnded(TaskId taskId);
    void updateTaskbar();

    std::unordered_map<TaskId, int> m_mapTaskIdProgress;
    QWinTaskbarButton* m_taskbarBtn = nullptr;
    int m_globalPct = 0;
};

} // namespace Mayo
