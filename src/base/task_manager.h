/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "signal.h"
#include "task_progress.h"

#include <functional>
#include <string>
#include <string_view>

namespace Mayo {

// Piece of code to be executed as a task(ie with TaskManager::run/exec())
using TaskJob = std::function<void(TaskProgress*)>;

// Central class providing creation/execution/deletion of Task objects
class TaskManager {
public:
    // Ctor & dtor
    TaskManager();
    ~TaskManager();

    // Not copyable
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    // Allocates a new task entity in manager
    // Returns the task identifier(unique in the scope of the owning TaskManager)
    TaskId newTask(TaskJob fn);

    // Asynchronous execution of job associated with task identifier 'id'
    // By default destroy policy is set to 'On' meaning the task will be deleted at some point
    // after its completion
    // This function is based on std::async() so depending on the C++ stdlib implementation this is
    // probably using a thread pool
    // NOTE The task must have been allocated previously with newTask()
    void run(TaskId id, TaskAutoDestroy policy = TaskAutoDestroy::On);

    // Same as run() but execution of the task job is synchronous(it runs in the current thread
    // just like a regular function call)
    void exec(TaskId id, TaskAutoDestroy policy = TaskAutoDestroy::On);

    // Current progress of task identified by 'id'
    // NOTE The task must have been allocated previously with newTask()
    int progress(TaskId id) const;

    // Current progress of all tasks
    int globalProgress() const;

    // Title(description) of a task identified by 'id'
    const std::string& title(TaskId id) const;
    void setTitle(TaskId id, std::string_view title);

    // Blocks the current thread until task of identifier 'id' has finished
    bool waitForDone(TaskId id, int msecs = -1);

    // Instructs the task of identifier 'id' to abort as soon as possible
    // Task interruption relies on the task job for this: it has to check regularly the
    // TaskProgress::isAbortRequested() flag and interrupt consequently
    void requestAbort(TaskId id);

    // Applies function 'fn' to each task
    void foreachTask(const std::function<void(TaskId)>& fn);

    // Signal emitted when some task execution has just started
    Signal<TaskId> signalStarted;

    // Signal emitted when the current step description of some task has changed
    Signal<TaskId, const std::string&> signalProgressStep;

    // Signal emitted when the current progress of some task has changed
    Signal<TaskId, int> signalProgressChanged;

    // Signal emitted when requestAbort() was called on some task
    Signal<TaskId> signalAbortRequested;

    // Signal emitted when some task execution has just ended(whatever stop condition: finished
    // or aborted)
    Signal<TaskId> signalEnded;

private:
    struct Entity;
    struct Private;
    Private* const d = nullptr;
};

} // namespace Mayo
