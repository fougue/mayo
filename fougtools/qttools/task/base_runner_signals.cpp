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

#include "base_runner_signals.h"

#include "base_runner.h"
#include "manager.h"

namespace qttask {

BaseRunnerSignals::BaseRunnerSignals(BaseRunner* runner, QObject *parent)
    : QObject(parent),
      m_runner(runner)
{
    QObject::connect(this, &BaseRunnerSignals::aboutToRun, runner->m_mgr, &Manager::onAboutToRun);
    QObject::connect(this, &BaseRunnerSignals::started, runner->m_mgr, &Manager::started);
    QObject::connect(this, &BaseRunnerSignals::progressStep, runner->m_mgr, &Manager::progressStep);
    QObject::connect(this, &BaseRunnerSignals::progress, runner->m_mgr, &Manager::progress);
    QObject::connect(this, &BaseRunnerSignals::message, runner->m_mgr, &Manager::message);
    QObject::connect(this, &BaseRunnerSignals::ended, runner->m_mgr, &Manager::ended);
    QObject::connect(this, &BaseRunnerSignals::destroyRequest, runner->m_mgr, &Manager::onDestroyRequest);
}

void BaseRunnerSignals::emitAboutToRun()
{
    emit aboutToRun(m_runner);
}

void BaseRunnerSignals::emitStarted(const QString &title)
{
    emit started(m_runner->m_taskId, title);
}

void BaseRunnerSignals::emitProgressStep(const QString &title)
{
    emit progressStep(m_runner->m_taskId, title);
}

void BaseRunnerSignals::emitProgress(int pct)
{
    emit progress(m_runner->m_taskId, pct);
}

void BaseRunnerSignals::emitMessage(const QString &msg)
{
    emit message(m_runner->m_taskId, msg);
}

void BaseRunnerSignals::emitEnded()
{
    emit ended(m_runner->m_taskId);
}

void BaseRunnerSignals::emitDestroyRequest()
{
    emit destroyRequest(m_runner);
}

} // namespace qttask
