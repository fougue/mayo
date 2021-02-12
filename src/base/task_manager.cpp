/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_manager.h"
#include "math_utils.h"

#include <QtCore/QCoreApplication>
#include <cassert>

namespace Mayo {

TaskManager::TaskManager(QObject* parent)
    : QObject(parent)
{
    static bool staticTypesRegistered = false;
    if (!staticTypesRegistered) {
        qRegisterMetaType<TaskId>("Mayo::TaskId");
        qRegisterMetaType<TaskId>("TaskId");
        staticTypesRegistered = true;
    }
}

TaskManager* TaskManager::globalInstance()
{
    static TaskManager* global = nullptr;
    if (!global)
        global = new TaskManager(QCoreApplication::instance());

    return global;
}

TaskId TaskManager::newTask(TaskJob fn)
{
    const TaskId taskId = m_taskIdSeq.fetch_add(1);
    std::unique_ptr<Entity> ptrEntity(new Entity);
    ptrEntity->task.m_id = taskId;
    ptrEntity->task.m_fn = std::move(fn);
    ptrEntity->task.m_manager = this;
    const TaskProgress progress(ptrEntity->task);
    ptrEntity->taskProgress = std::move(progress);
    ptrEntity->isGarbage = false;
    m_mapEntity.insert({ taskId, std::move(ptrEntity) });
    return taskId;
}

void TaskManager::run(TaskId id, TaskAutoDestroy autoDestroy)
{
    this->cleanGarbage();
    Entity* entity = this->findEntity(id);
    if (!entity)
        return;

    entity->control = std::async([=]{
        emit this->started(id);
        const TaskJob& fn = entity->task.job();
        fn(&entity->taskProgress);
        emit this->ended(id);
        if (autoDestroy == TaskAutoDestroy::On) {
            entity->isGarbage = true;
        }
    });
}

bool TaskManager::waitForDone(TaskId id, int msecs)
{
    Entity* entity = this->findEntity(id);
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
    Entity* entity = this->findEntity(id);
    if (entity) {
        emit this->abortRequested(id);
        entity->taskProgress.requestAbort();
    }
}

int TaskManager::progress(TaskId id) const
{
    const Entity* entity = this->findEntity(id);
    return entity ? entity->taskProgress.value() : 0;
}

int TaskManager::globalProgress() const
{
    int taskAccumPct = 0;
    for (const auto& mapPair : m_mapEntity) {
        const std::unique_ptr<Entity>& ptrEntity = mapPair.second;
        if (ptrEntity->taskProgress.value())
            taskAccumPct += ptrEntity->taskProgress.value();
    }

    const int taskCount = m_mapEntity.size();
    const int newGlobalPct = MathUtils::mappedValue(taskAccumPct, 0, taskCount * 100, 0, 100);
    return newGlobalPct;
}

QString TaskManager::title(TaskId id) const
{
    const Entity* entity = this->findEntity(id);
    return entity ? entity->title : QString();
}

void TaskManager::setTitle(TaskId id, const QString& title)
{
    Entity* entity = this->findEntity(id);
    if (entity)
        entity->title = title;
}

TaskManager::Entity* TaskManager::findEntity(TaskId id)
{
    auto it = m_mapEntity.find(id);
    return it != m_mapEntity.end() ? it->second.get() : nullptr;
}

const TaskManager::Entity* TaskManager::findEntity(TaskId id) const
{
    auto it = m_mapEntity.find(id);
    return it != m_mapEntity.cend() ? it->second.get() : nullptr;
}

void TaskManager::cleanGarbage()
{
    auto it = m_mapEntity.begin();
    while (it != m_mapEntity.end()) {
        Entity* entity = it->second.get();
        if (entity->isGarbage.load())
            it = m_mapEntity.erase(it);
        else
            ++it;
    }
}

} // namespace Mayo
