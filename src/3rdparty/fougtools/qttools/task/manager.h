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

#include "runner_qthread.h"

#include <QtCore/QObject>

#include <atomic>
#include <unordered_map>

namespace qttask {

class BaseRunner;
class Progress;

/*! \brief Central class providing management of tasks and notifications
 */
class Manager : public QObject
{
    Q_OBJECT

public:
    Manager(QObject* parent = nullptr);
    ~Manager();

    /*! \brief Create a ready-to-launch Runner object
     *
     *  Typical use:
     *  \code
     *      auto task = qttask::Manager::globalInstance()->newTask();
     *      task->run( [=] { someFunction(task->progress()); } );
     *  \endcode
     *
     *  The created Runner object will be automatically deleted at the end
     *  of BaseRunner::run().
     *  If for any reason BaseRunner::run() is not called, the Runner object
     *  has to be deleted by the caller.
     */
    template<typename SELECTOR = QThread, typename ... ARGS>
    Runner<SELECTOR>* newTask(ARGS ... args)
    {
        auto runner = new Runner<SELECTOR>(this, args ...);
        runner->m_taskId = m_taskIdSeq.fetch_add(1);
        return runner;
    }

    QString taskTitle(quint64 taskId) const;
    const Progress* taskProgress(quint64 taskId) const;

    void requestAbort(quint64 taskId);

    static Manager* globalInstance();

signals:
    void started(quint64 taskId, const QString& title);
    void progressStep(quint64 taskId, const QString& title);
    void progress(quint64 taskId, int percent);
    void message(quint64 taskId, const QString& msg);
    void ended(quint64 taskId);

private:
    friend class BaseRunnerSignals;

    void onAboutToRun(BaseRunner* runner);
    void onDestroyRequest(BaseRunner* runner);
    BaseRunner* getRunner(quint64 taskId);
    const BaseRunner* getRunner(quint64 taskId) const;

    std::atomic<quint64> m_taskIdSeq = {};
    std::unordered_map<quint64, BaseRunner*> m_taskIdToRunner;
};

} // namespace qttask
