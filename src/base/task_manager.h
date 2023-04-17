/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "signal.h"
#include "task.h"
#include "task_progress.h"

#include <functional>
#include <string>
#include <string_view>

namespace Mayo {

// Central class providing management of Task objects
class TaskManager {
public:
    TaskManager();
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

    void foreachTask(const std::function<void(TaskId)>& fn);

    // Signals
    Signal<TaskId> signalStarted;
    Signal<TaskId, const std::string&> signalProgressStep;
    Signal<TaskId, int> signalProgressChanged;
    Signal<TaskId> signalAbortRequested;
    Signal<TaskId> signalEnded;

private:
    struct Entity;
    struct Private;
    Private* const d = nullptr;
};

} // namespace Mayo
