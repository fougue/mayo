/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/task_manager.h"
#include "../src/base/task_progress.h"
#include "signal_emit_spy.h"

#include <stdexcept>
#include <thread>

namespace Mayo {

void TestBase::LibTask_runAndTrackProgress_test()
{
    struct ProgressRecord {
        TaskId taskId;
        double value;
    };

    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([=](TaskProgress* progress) {
        TaskProgress subProgress1(progress, 40);
        for (int i = 0; i <= 100; ++i)
            subProgress1.setValue(i);

        TaskProgress subProgress2(progress, 60);
        for (int i = 0; i <= 100; ++i)
            subProgress2.setValue(i);
    });
    std::vector<ProgressRecord> vecProgressRec;
    taskMgr.signalProgressChanged.connectSlot([&](TaskId taskId, double pct) {
        vecProgressRec.push_back({ taskId, pct });
    });

    SignalEmitSpy sigStarted(&taskMgr.signalStarted);
    SignalEmitSpy sigEnded(&taskMgr.signalEnded);
    taskMgr.run(taskId);
    taskMgr.waitForDone(taskId);

    QCOMPARE(sigStarted.count, 1);
    QCOMPARE(sigEnded.count, 1);
    QCOMPARE(std::get<uint64_t>(sigStarted.vecSignals.front().at(0)), taskId);
    QCOMPARE(std::get<uint64_t>(sigEnded.vecSignals.front().at(0)), taskId);
    QVERIFY(!vecProgressRec.empty());
    int prevPct = 0;
    for (const ProgressRecord& rec : vecProgressRec) {
        QCOMPARE(rec.taskId, taskId);
        QVERIFY(prevPct <= rec.value);
        prevPct = rec.value;
    }

    QCOMPARE(vecProgressRec.front().value, 0.);
    QCOMPARE(vecProgressRec.back().value, 100.);
}

void TestBase::LibTask_runJobException_test()
{
    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([](TaskProgress*) {
        throw std::runtime_error("Test exception");
    });

    SignalEmitSpy sigStarted(&taskMgr.signalStarted);
    SignalEmitSpy sigEnded(&taskMgr.signalEnded);

    taskMgr.run(taskId, TaskAutoDestroy::Off);

    QVERIFY(taskMgr.waitForDone(taskId, 250/*ms*/));

    QCOMPARE(sigStarted.count, 1);
    QCOMPARE(sigEnded.count, 1);
}

void TestBase::LibTask_taskCompleted_test()
{
    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([](TaskProgress* progress) {
        progress->setValue(50.);
    });

    SignalEmitSpy spy(&taskMgr.signalEnded);
    taskMgr.run(taskId, TaskAutoDestroy::Off);

    QVERIFY(taskMgr.waitForDone(taskId));

    QCOMPARE(spy.count, 1);
    QCOMPARE(spy.getArgAs<TaskId>(0, 0), taskId);
    QCOMPARE(spy.getArgAs<TaskEndReason>(0, 1), TaskEndReason::Completed);
    QCOMPARE(taskMgr.progress(taskId), 100.);
}

void TestBase::LibTask_taskAborted_test()
{
    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([](TaskProgress* progress) {
        progress->setValue(0.);
        progress->setValue(42.);
        while (!progress->isAbortRequested())
            std::this_thread::yield();
    });
    taskMgr.signalProgressChanged.connectSlot([&taskMgr](TaskId taskId, double progress) {
        if (progress > 0.)
            taskMgr.requestAbort(taskId);
    });

    SignalEmitSpy spy(&taskMgr.signalEnded);
    taskMgr.run(taskId, TaskAutoDestroy::Off);

    QVERIFY(taskMgr.waitForDone(taskId));

    QCOMPARE(spy.count, 1);
    QCOMPARE(spy.getArgAs<TaskId>(0, 0), taskId);
    QCOMPARE(spy.getArgAs<TaskEndReason>(0, 1), TaskEndReason::Aborted);
    QCOMPARE(taskMgr.progress(taskId), 42.);
}

void TestBase::LibTask_taskFailed_test()
{
    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([](TaskProgress*) {
        throw std::runtime_error("Unexpected failure");
    });

    SignalEmitSpy spy(&taskMgr.signalEnded);
    taskMgr.run(taskId, TaskAutoDestroy::Off);

    // The exception must not escape from the task execution function
    QVERIFY(taskMgr.waitForDone(taskId));

    // signalEnded() must still be emitted
    QCOMPARE(spy.count, 1);
    QCOMPARE(spy.getArgAs<TaskId>(0, 0), taskId);
    QCOMPARE(spy.getArgAs<TaskEndReason>(0, 1), TaskEndReason::Failed);
}

void TestBase::LibTask_TaskProgress_isRoot_test()
{
    TaskProgress rootProgress;
    QVERIFY(rootProgress.isRoot());

    TaskProgress childProgress(&rootProgress, 50.);
    QVERIFY(!childProgress.isRoot());

    TaskProgress grandchildProgress(&childProgress, 50.);
    QVERIFY(!grandchildProgress.isRoot());
}

void TestBase::LibTask_TaskManager_destroy_test()
{
    TaskManager taskMgr;

    QVERIFY(!taskMgr.destroy(TaskId_null));

    const TaskId taskId = taskMgr.newTask([](TaskProgress*) {});
    QVERIFY(!taskMgr.destroy(taskId)); // The task has not finished yet

    taskMgr.exec(taskId, TaskAutoDestroy::Off);
    QVERIFY(taskMgr.destroy(taskId));
    QVERIFY(!taskMgr.destroy(taskId)); // The task has already been destroyed

    const TaskId asyncTaskId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.run(asyncTaskId, TaskAutoDestroy::Off);
    QVERIFY(taskMgr.waitForDone(asyncTaskId));
    QVERIFY(taskMgr.destroy(asyncTaskId));
}

void TestBase::LibTask_TaskManager_title_test()
{
    TaskManager taskMgr;
    QVERIFY(taskMgr.title(TaskId_null).empty());

    const TaskId taskId = taskMgr.newTask([](TaskProgress*) {});
    QVERIFY(taskMgr.title(taskId).empty());
}

void TestBase::LibTask_TaskManager_setTitle_test()
{
    TaskManager taskMgr;
    const TaskId taskId = taskMgr.newTask([](TaskProgress*) {});

    taskMgr.setTitle(taskId, "Initial title");
    QVERIFY(taskMgr.title(taskId) == "Initial title");

    taskMgr.setTitle(taskId, "Updated title");
    QVERIFY(taskMgr.title(taskId) == "Updated title");

    taskMgr.setTitle(TaskId_null, "Missing task");
    QVERIFY(taskMgr.title(taskId) == "Updated title");
}

void TestBase::LibTask_TaskManager_cleanGarbage_test()
{
    TaskManager taskMgr;

    // A synchronously completed auto-destroy task is removed by the next execution
    const TaskId syncAutoDestroyId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.exec(syncAutoDestroyId, TaskAutoDestroy::On);

    const TaskId retainedId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.setTitle(retainedId, "Retained");
    taskMgr.exec(retainedId, TaskAutoDestroy::Off);
    QVERIFY(taskMgr.title(syncAutoDestroyId).empty());

    // An asynchronously completed auto-destroy task is also collected
    const TaskId asyncAutoDestroyId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.run(asyncAutoDestroyId, TaskAutoDestroy::On);
    QVERIFY(taskMgr.waitForDone(asyncAutoDestroyId));

    // Garbage collection keeps completed non-auto-destroy and unfinished tasks
    const TaskId unfinishedId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.setTitle(unfinishedId, "Unfinished");
    const TaskId triggerId = taskMgr.newTask([](TaskProgress*) {});
    taskMgr.exec(triggerId, TaskAutoDestroy::Off);

    QVERIFY(taskMgr.title(asyncAutoDestroyId).empty());
    QVERIFY(taskMgr.title(retainedId) == "Retained");
    QVERIFY(taskMgr.title(unfinishedId) == "Unfinished");
    QVERIFY(taskMgr.destroy(retainedId));
    QVERIFY(!taskMgr.destroy(unfinishedId));
}

} // namespace Mayo
