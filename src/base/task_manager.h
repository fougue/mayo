/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "signal.h"
#include "task.h"
#include "task_progress.h"

#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Mayo {

class TaskManager {
public:
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

    template<typename Function>
    void foreachTask(Function fn) {
        for (const auto& mapPair : m_mapEntity)
            fn(mapPair.first);
    }

    // Signals
    Signal<TaskId> signalStarted;
    Signal<TaskId, const std::string&> signalProgressStep;
    Signal<TaskId, int> signalProgressChanged;
    Signal<TaskId> signalAbortRequested;
    Signal<TaskId> signalEnded;

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
