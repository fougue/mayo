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

#include "manager.h"

#include "base_runner.h"

#include <QtCore/QGlobalStatic>

namespace qttask {

Manager::Manager(QObject *parent)
    : QObject(parent)
{
}

Manager::~Manager()
{ }

QString Manager::taskTitle(quint64 taskId) const
{
    auto runner = this->getRunner(taskId);
    return runner != nullptr ? runner->taskTitle() : QString();
}

const Progress *Manager::taskProgress(quint64 taskId) const
{
    auto runner = this->getRunner(taskId);
    return runner != nullptr ? &runner->progress() : nullptr;
}

void Manager::requestAbort(quint64 taskId)
{
    auto runner = this->getRunner(taskId);
    if (runner != nullptr)
        runner->requestAbort();
}

Q_GLOBAL_STATIC(Manager, mgrGlobalInstance)

Manager *Manager::globalInstance()
{
    return mgrGlobalInstance();
}

void Manager::onAboutToRun(BaseRunner *runner)
{
    m_taskIdToRunner.emplace(runner->m_taskId, runner);
}

void Manager::onDestroyRequest(BaseRunner *runner)
{
    m_taskIdToRunner.erase(runner->taskId());
    runner->destroy();
}

BaseRunner *Manager::getRunner(quint64 taskId)
{
    return const_cast<BaseRunner*>(static_cast<const Manager*>(this)->getRunner(taskId));
}

const BaseRunner *Manager::getRunner(quint64 taskId) const
{
    auto it = m_taskIdToRunner.find(taskId);
    return it != m_taskIdToRunner.end() ? (*it).second : nullptr;
}

} // namespace qttask
