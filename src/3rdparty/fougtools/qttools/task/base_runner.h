/****************************************************************************
**  FougTools
**  Copyright Fougue (30 Mar. 2015)
**  contact@fougue.pro
**
** This software is a computer program whose purpose is to provide utility
** tools for the C++ language and the Qt toolkit.
**
** This software is governed by the CeCILL-C license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-C
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
****************************************************************************/

#pragma once

#include "progress.h"
#include "base_runner_signals.h"

#include <functional>

namespace qttask {

class Manager;

/*! \brief Base class for all runner objects
 *
 *  BaseRunner provides control over a Runnable object : start/abort
 */
class BaseRunner
{
public:
    virtual ~BaseRunner();

    quint64 taskId() const;

    const QString& taskTitle() const;
    void setTaskTitle(const QString& title);

    Progress& progress();
    const Progress& progress() const;

    void run(std::function<void()> func);

    virtual bool isAbortRequested() const;
    virtual void requestAbort();

protected:
    BaseRunner(const Manager* mgr);

    BaseRunnerSignals* qtSignals();

    void execRunnableFunc();

    virtual void launch();
    virtual void destroy();

private:
    friend class BaseRunnerSignals;
    friend class Manager;
    friend class Progress;

    const Manager* m_mgr = nullptr;
    quint64 m_taskId = 0;
    QString m_taskTitle;
    std::function<void()> m_func;

    BaseRunnerSignals m_signals;
    Progress m_progress;
};

template<typename SELECTOR>
class Runner : public BaseRunner
{ };

} // namespace qttask
