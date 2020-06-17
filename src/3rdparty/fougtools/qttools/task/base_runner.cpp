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

#include "base_runner.h"

#include "manager.h"

namespace qttask {

BaseRunner::BaseRunner(const Manager* mgr)
    : m_mgr(mgr),
      m_signals(this),
      m_progress(this)
{ }

BaseRunner::~BaseRunner()
{ }

quint64 BaseRunner::taskId() const
{
    return m_taskId;
}

const QString& BaseRunner::taskTitle() const
{
    return m_taskTitle;
}

void BaseRunner::setTaskTitle(const QString& title)
{
    m_taskTitle = title;
}

Progress& BaseRunner::progress()
{
    return m_progress;
}

const Progress& BaseRunner::progress() const
{
    return m_progress;
}

void BaseRunner::run(std::function<void()> func)
{
    m_func = std::move(func);
    if (m_func) {
        m_signals.emitAboutToRun();
        this->launch();
    }
}

BaseRunnerSignals *BaseRunner::qtSignals()
{
    return &m_signals;
}

void BaseRunner::execRunnableFunc()
{
    m_signals.emitStarted(m_taskTitle);
    m_func();
    m_signals.emitEnded();
    m_signals.emitDestroyRequest();
}

bool BaseRunner::isAbortRequested() const
{
    return false;
}

void BaseRunner::requestAbort()
{ }

void BaseRunner::launch()
{ }

void BaseRunner::destroy()
{
    delete this;
}

} // namespace qttask
