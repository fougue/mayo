/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "task_manager.h"

#include "application.h"
#include "cpp_utils.h"
#include "math_utils.h"

#include <cassert>

namespace Mayo {

TaskManager::TaskManager(QObject* parent)
    : QObject(parent)
{
}

TaskManager::~TaskManager()
{
    // Make sure all tasks are really finished
    for (const auto& mapPair : m_mapEntity) {
        const std::unique_ptr<Entity>& ptrEntity = mapPair.second;
        if (ptrEntity->control.valid())
            ptrEntity->control.wait();
    }

    // Erase the task from its container before destruction, this will allow TaskProgress destructor
    // to behave correctly(it calls TaskProgress::setValue())
    for (auto it = m_mapEntity.begin(); it != m_mapEntity.end(); )
        it = m_mapEntity.erase(it);
}

TaskManager* TaskManager::globalInstance()
{
    static TaskManager* global = nullptr;
    if (!global)
        global = new TaskManager(Application::instance().get());

    return global;
}

TaskId TaskManager::newTask(TaskJob fn)
{
    const TaskId taskId = m_taskIdSeq.fetch_add(1);
    std::unique_ptr<Entity> ptrEntity(new Entity);
    ptrEntity->task.m_id = taskId;
    ptrEntity->task.m_fn = std::move(fn);
    ptrEntity->task.m_manager = this;
    ptrEntity->taskProgress.setTask(&ptrEntity->task);
    m_mapEntity.insert({ taskId, std::move(ptrEntity) });
    return taskId;
}

void TaskManager::run(TaskId id, TaskAutoDestroy policy)
{
    this->cleanGarbage();
    Entity* entity = this->findEntity(id);
    if (!entity)
        return;

    entity->isFinished = false;
    entity->autoDestroy = policy;
    entity->control = std::async([=]{ this->execEntity(entity); });
}

void TaskManager::exec(TaskId id, TaskAutoDestroy policy)
{
    this->cleanGarbage();
    Entity* entity = this->findEntity(id);
    if (!entity)
        return;

    entity->isFinished = false;
    entity->autoDestroy = policy;
    this->execEntity(entity);
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
        if (ptrEntity->taskProgress.value() > 0)
            taskAccumPct += ptrEntity->taskProgress.value();
    }

    const int taskCount = m_mapEntity.size();
    const int newGlobalPct = MathUtils::mappedValue(taskAccumPct, 0, taskCount * 100, 0, 100);
    //qDebug() << "taskCount=" << taskCount << " taskAccumPct=" << taskAccumPct << " newGlobalPct=" << newGlobalPct;
    return newGlobalPct;
}

const std::string& TaskManager::title(TaskId id) const
{
    const Entity* entity = this->findEntity(id);
    return entity ? entity->title : CppUtils::nullString();
}

void TaskManager::setTitle(TaskId id, std::string_view title)
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

void TaskManager::execEntity(Entity* entity)
{
    if (!entity)
        return;

    emit this->started(entity->task.id());
    const TaskJob& fn = entity->task.job();
    fn(&entity->taskProgress);
    if (!entity->taskProgress.isAbortRequested())
        entity->taskProgress.setValue(100);

    emit this->ended(entity->task.id());
    entity->isFinished = true;
}

void TaskManager::cleanGarbage()
{
    auto it = m_mapEntity.begin();
    while (it != m_mapEntity.end()) {
        Entity* entity = it->second.get();
        if (entity->isFinished && entity->autoDestroy == TaskAutoDestroy::On) {
            if (entity->control.valid())
                entity->control.wait();

            it = m_mapEntity.erase(it);
        }
        else {
            ++it;
        }
    }
}

} // namespace Mayo
