/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task.h"
#include "task_progress.h"

#include <QtCore/QObject>
#include <atomic>
#include <future>
#include <memory>
#include <unordered_map>

namespace Mayo {

class TaskManager : public QObject {
    Q_OBJECT
public:
    TaskManager(QObject* parent = nullptr);
    static TaskManager* globalInstance();

    TaskId newTask(TaskJob fn);
    void run(TaskId id, TaskAutoDestroy autoDestroy = TaskAutoDestroy::On);

    int progress(TaskId id) const;
    int globalProgress() const;

    QString title(TaskId id) const;
    void setTitle(TaskId id, const QString& title);

    bool waitForDone(TaskId id, int msecs = -1);
    void requestAbort(TaskId id);

signals:
    void started(TaskId id);
    void progressStep(TaskId id, const QString& stepTitle);
    void progressChanged(TaskId id, int percent);
    void abortRequested(TaskId id);
    void ended(TaskId id);

private:
    struct Entity {
        Task task;
        TaskProgress taskProgress;
        QString title;
        std::future<void> control;
        std::atomic<bool> isGarbage;
    };

    Entity* findEntity(TaskId id);
    const Entity* findEntity(TaskId id) const;
    void cleanGarbage();

    std::atomic<TaskId> m_taskIdSeq = {};
    std::unordered_map<TaskId, std::unique_ptr<Entity>> m_mapEntity;
};

} // namespace Mayo
