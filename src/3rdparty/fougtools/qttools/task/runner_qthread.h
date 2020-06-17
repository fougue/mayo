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

#include "base_runner.h"

#include <QtCore/QThread>
#include <QtCore/QTimer>

namespace qttask {

/*! \brief Task runner based on QThread
 */
template<>
class Runner<QThread> : public QThread, public BaseRunner {
public:
    Runner<QThread>(const Manager* mgr, QThread::Priority priority = QThread::InheritPriority)
        : QThread(nullptr),
          BaseRunner(mgr),
          m_priority(priority)
    {}

    bool isAbortRequested() const override {
        return this->isInterruptionRequested();
    }

    void requestAbort() override {
        this->requestInterruption();
    }

protected:
    void launch() override {
        this->start(m_priority);
    }

    void destroy() override {
        this->deleteLater();
    }

    void run() override { // -- QThread
        QTimer::singleShot(0, [=] {
            this->execRunnableFunc();
            this->quit();
        });

        this->exec();
    }

private:
    QThread::Priority m_priority;
};

} // namespace qttask
