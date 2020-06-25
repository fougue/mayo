/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "task_common.h"
#include <QtCore/QString>

namespace Mayo {

class Task;

class TaskProgress {
public:
    TaskId taskId() const;

    int value() const { return m_value; }
    void setValue(int pct);

    const QString& step() const { return m_step; }
    void setStep(const QString& title);

    bool isAbortRequested() const { return m_isAbortRequested; }

    void beginScope(int scopeSize, const QString& stepTitle = QString());
    void endScope();

private:
    TaskProgress() = default;
    TaskProgress(const Task& task);

    void requestAbort();

    friend class TaskManager;
    const Task* m_task;
    int m_value = 0;
    QString m_step;
    int m_currentScopeSize = -1;
    int m_currentScopeValueStart = 0;
    bool m_isAbortRequested = false;
};

} // namespace Mayo
