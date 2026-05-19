/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
    void onTaskProgress(TaskId taskId, double percent);
    void onTaskEnded(TaskId taskId);
    void updateTaskbar();

    std::unordered_map<TaskId, double> m_mapTaskIdProgress;
    QWinTaskbarButton* m_taskbarBtn = nullptr;
    double m_globalPct = 0.;
};

} // namespace Mayo
