/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "win_taskbar_global_progress.h"

#include "../base/math_utils.h"

#include <fougtools/qttools/task/manager.h>
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>

namespace Mayo {

WinTaskbarGlobalProgress::WinTaskbarGlobalProgress(QObject* parent)
    : QObject(parent),
      m_taskbarBtn(new QWinTaskbarButton(this))
{
    auto taskMgr = qttask::Manager::globalInstance();
    QObject::connect(
                taskMgr, &qttask::Manager::started,
                this, [=](quint64 taskId) { this->onTaskProgress(taskId, 0); });
    QObject::connect(
                taskMgr, &qttask::Manager::progress,
                this, &WinTaskbarGlobalProgress::onTaskProgress);
    QObject::connect(
                taskMgr, &qttask::Manager::ended,
                this, &WinTaskbarGlobalProgress::onTaskEnded);
}

void WinTaskbarGlobalProgress::setWindow(QWindow* window)
{
    m_taskbarBtn->setWindow(window);
}

void WinTaskbarGlobalProgress::onTaskProgress(quint64 taskId, int percent)
{
    auto it = m_mapTaskIdProgress.find(taskId);
    if (it != m_mapTaskIdProgress.end())
        it->second = percent;
    else
        m_mapTaskIdProgress.insert({ taskId, percent });

    this->updateTaskbar();
}

void WinTaskbarGlobalProgress::onTaskEnded(quint64 taskId)
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
