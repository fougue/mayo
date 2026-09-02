/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "test_base.h"

#include "../src/base/task_manager.h"
#include "../src/base/task_progress.h"
#include "signal_emit_spy.h"

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
    QCOMPARE(std::get<TaskId>(sigStarted.vecSignals.front().at(0)), taskId);
    QCOMPARE(std::get<TaskId>(sigEnded.vecSignals.front().at(0)), taskId);
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

} // namespace Mayo
