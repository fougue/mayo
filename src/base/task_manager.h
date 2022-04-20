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
#include <string>
#include <string_view>
#include <unordered_map>

namespace Mayo {

class TaskManager : public QObject {
    Q_OBJECT
public:
    TaskManager(QObject* parent = nullptr);
    ~TaskManager();

    TaskId newTask(TaskJob fn);
    void run(TaskId id, TaskAutoDestroy policy = TaskAutoDestroy::On);
    void exec(TaskId id, TaskAutoDestroy policy = TaskAutoDestroy::On); // Synchronous

    int progress(TaskId id) const;
    int globalProgress() const;

    const std::string& title(TaskId id) const;
    void setTitle(TaskId id, std::string_view title);

    bool waitForDone(TaskId id, int msecs = -1);
    void requestAbort(TaskId id);

    template<typename FUNCTION>
    void foreachTask(FUNCTION fn) {
        for (const auto& mapPair : m_mapEntity)
            fn(mapPair.first);
    }

signals:
    void started(Mayo::TaskId id);
    void progressStep(Mayo::TaskId id, const std::string& stepTitle);
    void progressChanged(Mayo::TaskId id, int percent);
    void abortRequested(Mayo::TaskId id);
    void ended(Mayo::TaskId id);

private:
    struct Entity {
        Task task;
        TaskProgress taskProgress;
        std::string title;
        std::future<void> control;
        std::atomic<bool> isFinished = false;
        TaskAutoDestroy autoDestroy = TaskAutoDestroy::On;
    };

    Entity* findEntity(TaskId id);
    const Entity* findEntity(TaskId id) const;
    void execEntity(Entity* entity);
    void cleanGarbage();

    std::atomic<TaskId> m_taskIdSeq = {};
    std::unordered_map<TaskId, std::unique_ptr<Entity>> m_mapEntity;
};

} // namespace Mayo
