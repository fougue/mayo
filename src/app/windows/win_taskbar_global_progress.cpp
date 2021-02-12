/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "win_taskbar_global_progress.h"

#include "../base/math_utils.h"
#include "../base/task_manager.h"

#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>

namespace Mayo {

WinTaskbarGlobalProgress::WinTaskbarGlobalProgress(const TaskManager* taskMgr, QObject* parent)
    : QObject(parent),
      m_taskbarBtn(new QWinTaskbarButton(this)),
      m_taskMgr(taskMgr)
{
    QObject::connect(
                taskMgr, &TaskManager::started,
                this, [=](TaskId taskId) { this->onTaskProgress(taskId, 0); });
    QObject::connect(
                taskMgr, &TaskManager::progressChanged,
                this, &WinTaskbarGlobalProgress::onTaskProgress);
    QObject::connect(
                taskMgr, &TaskManager::ended,
                this, &WinTaskbarGlobalProgress::onTaskEnded);
}

void WinTaskbarGlobalProgress::setWindow(QWindow* window)
{
    m_taskbarBtn->setWindow(window);
}

void WinTaskbarGlobalProgress::onTaskProgress(TaskId taskId, int percent)
{
    auto it = m_mapTaskIdProgress.find(taskId);
    if (it != m_mapTaskIdProgress.end())
        it->second = percent;
    else
        m_mapTaskIdProgress.insert({ taskId, percent });

    this->updateTaskbar();
}

void WinTaskbarGlobalProgress::onTaskEnded(TaskId taskId)
{
    m_mapTaskIdProgress.erase(taskId);
    this->updateTaskbar();
}

void WinTaskbarGlobalProgress::updateTaskbar()
{
    QWinTaskbarProgress* taskbarProgress = m_taskbarBtn->progress();
    if (m_mapTaskIdProgress.empty()) {
        taskbarProgress->stop();
        taskbarProgress->hide();
        m_globalPct = 0;
        return;
    }

    int taskCount = 0;
    int taskAccumPct = 0;
    bool isProgressIndeterminate = false;
    for (const auto& mapPair : m_mapTaskIdProgress) {
        const int pct = mapPair.second;
        if (pct >= 0)
            taskAccumPct += pct;
        else
            isProgressIndeterminate = true;

        ++taskCount;
    }

    taskbarProgress->show();
    taskbarProgress->resume();
    if (!isProgressIndeterminate) {
        const int newGlobalPct = MathUtils::mappedValue(taskAccumPct, 0, taskCount * 100, 0, 100);
        m_globalPct = std::max(newGlobalPct, m_globalPct);
        taskbarProgress->setRange(0, 100);
        taskbarProgress->setValue(m_globalPct);
    }
    else {
        m_globalPct = 0;
        taskbarProgress->setRange(0, 0);
    }
}

} // namespace Mayo
