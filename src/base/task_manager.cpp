/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_manager.h"

#include "cpp_utils.h"
#include "math_utils.h"

#include <atomic>
#include <cassert>
#include <cmath>
#include <future>
#include <memory>
#include <unordered_map>

namespace Mayo {

// Helper struct providing all required data to manage a Task object
struct TaskManager::Entity {
    TaskId taskId;
    TaskJob taskJob;
    TaskProgress taskProgress;
    std::string title;
    std::future<void> control;
    std::atomic<bool> isFinished = false;
    TaskAutoDestroy autoDestroy = TaskAutoDestroy::On;
};

// Pimpl struct providing private(hidden) interface of TaskManager class
struct TaskManager::Private {
    // Ctor
    Private(TaskManager* mgr) : taskMgr(mgr) {}

    // Const/mutable functions to find an Entity from a task identifier. Returns null if not found
    TaskManager::Entity* findEntity(TaskId id);
    const TaskManager::Entity* findEntity(TaskId id) const;

    // Execute(synchronous) task entity, sending started/ended signals accordingly
    void execEntity(TaskManager::Entity* entity);

    // Destroy finished task entities whose policy was set to TaskAutoDestroy::On
    void cleanGarbage();

    TaskManager* taskMgr = nullptr;
    std::atomic<TaskId> taskIdSeq = {};
    std::unordered_map<TaskId, std::unique_ptr<TaskManager::Entity>> mapEntity;
};

TaskManager::TaskManager()
    : d(new Private(this))
{
}

TaskManager::~TaskManager()
{
    // Make sure all tasks are really finished
    for (const auto& mapPair : d->mapEntity) {
        const std::unique_ptr<Entity>& ptrEntity = mapPair.second;
        if (ptrEntity->control.valid())
            ptrEntity->control.wait();
    }

    // Erase the task from its container before destruction, this will allow TaskProgress destructor
    // to behave correctly(it calls TaskProgress::setValue())
    for (auto it = d->mapEntity.begin(); it != d->mapEntity.end(); )
        it = d->mapEntity.erase(it);

    delete d;
}

TaskId TaskManager::newTask(TaskJob fn)
{
    const TaskId taskId = d->taskIdSeq.fetch_add(1);
    std::unique_ptr<Entity> ptrEntity(new Entity);
    ptrEntity->taskId = taskId;
    ptrEntity->taskJob = std::move(fn);
    ptrEntity->taskProgress.setTaskId(taskId);
    ptrEntity->taskProgress.setTaskManager(this);
    d->mapEntity.insert({ taskId, std::move(ptrEntity) });
    return taskId;
}

void TaskManager::run(TaskId id, TaskAutoDestroy policy)
{
    d->cleanGarbage();
    Entity* entity = d->findEntity(id);
    if (!entity)
        return;

    entity->isFinished = false;
    entity->autoDestroy = policy;
    entity->control = std::async([=]{ d->execEntity(entity); });
}

void TaskManager::exec(TaskId id, TaskAutoDestroy policy)
{
    d->cleanGarbage();
    Entity* entity = d->findEntity(id);
    if (!entity)
        return;

    entity->isFinished = false;
    entity->autoDestroy = policy;
    d->execEntity(entity);
}

bool TaskManager::waitForDone(TaskId id, int msecs)
{
    Entity* entity = d->findEntity(id);
    if (!entity)
        return true;

    if (!entity->control.valid())
        return true;

    if (msecs < 0) {
        entity->control.wait();
        return true;
    }

    return entity->control.wait_for(std::chrono::milliseconds(msecs)) == std::future_status::ready;
}

void TaskManager::requestAbort(TaskId id)
{
    Entity* entity = d->findEntity(id);
    if (entity) {
        this->signalAbortRequested.send(id);
        entity->taskProgress.requestAbort();
    }
}

void TaskManager::foreachTask(const std::function<void(TaskId)>& fn)
{
    for (const auto& mapPair : d->mapEntity)
        fn(mapPair.first);
}

int TaskManager::progress(TaskId id) const
{
    const Entity* entity = d->findEntity(id);
    return entity ? entity->taskProgress.value() : 0;
}

int TaskManager::globalProgress() const
{
    int taskAccumPct = 0;
    for (const auto& mapPair : d->mapEntity) {
        const std::unique_ptr<Entity>& ptrEntity = mapPair.second;
        if (ptrEntity->taskProgress.value() > 0)
            taskAccumPct += ptrEntity->taskProgress.value();
    }

    const auto pct = MathUtils::toPercent(taskAccumPct, 0, d->mapEntity.size() * 100);
    return std::lround(pct);
}

const std::string& TaskManager::title(TaskId id) const
{
    const Entity* entity = d->findEntity(id);
    return entity ? entity->title : CppUtils::nullString();
}

void TaskManager::setTitle(TaskId id, std::string_view title)
{
    Entity* entity = d->findEntity(id);
    if (entity)
        entity->title = title;
}

TaskManager::Entity* TaskManager::Private::findEntity(TaskId id)
{
    auto it = this->mapEntity.find(id);
    return it != this->mapEntity.end() ? it->second.get() : nullptr;
}

const TaskManager::Entity* TaskManager::Private::findEntity(TaskId id) const
{
    auto it = this->mapEntity.find(id);
    return it != this->mapEntity.cend() ? it->second.get() : nullptr;
}

void TaskManager::Private::execEntity(Entity* entity)
{
    if (!entity)
        return;

    this->taskMgr->signalStarted.send(entity->taskId);
    const TaskJob& fn = entity->taskJob;
    fn(&entity->taskProgress);
    if (!entity->taskProgress.isAbortRequested())
        entity->taskProgress.setValue(100);

    this->taskMgr->signalEnded.send(entity->taskId);
    entity->isFinished = true;
}

void TaskManager::Private::cleanGarbage()
{
    auto it = this->mapEntity.begin();
    while (it != this->mapEntity.end()) {
        Entity* entity = it->second.get();
        if (entity->isFinished && entity->autoDestroy == TaskAutoDestroy::On) {
            if (entity->control.valid())
                entity->control.wait();

            it = this->mapEntity.erase(it);
        }
        else {
            ++it;
        }
    }
}

} // namespace Mayo
