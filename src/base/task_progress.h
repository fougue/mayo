/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
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
    TaskProgress() = default;

    TaskId taskId() const;

    int value() const { return m_value; }
    void setValue(int pct);

    const QString& step() const { return m_step; }
    void setStep(const QString& title);

    bool isAbortRequested() const { return m_isAbortRequested; }
    static bool isAbortRequested(const TaskProgress* progress);

    void beginScope(int scopeSize, const QString& stepTitle = QString());
    void endScope();

private:
    TaskProgress(const Task& task);

    void requestAbort();

    friend class TaskManager;
    const Task* m_task = nullptr;
    int m_value = 0;
    QString m_step;
    int m_currentScopeSize = -1;
    int m_currentScopeValueStart = 0;
    bool m_isAbortRequested = false;
};

} // namespace Mayo
