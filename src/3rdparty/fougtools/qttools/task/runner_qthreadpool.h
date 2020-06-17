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

#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>

namespace qttask {

/*! \brief Task runner using the global instance of QThreadPool
 *
 *  Internally using QThreadPool::globalInstance()->start(...)
 */
template<>
class Runner<QThreadPool> : public QRunnable, public BaseRunner {
public:
    /*! \param priority Same meaning as the second parameter of
     *                  QThreadPool::start(QRunnable*, int priority)
     */
    Runner<QThreadPool>(const Manager* mgr, int priority = 0)
        : BaseRunner(mgr),
          m_priority(priority)
    {
        this->setAutoDelete(false);
    }

    void run() override { // -- QRunnable
        this->execRunnableFunc();
    }

    bool isAbortRequested() const override {
        return m_isAbortRequested;
    }

    void requestAbort() override {
        m_isAbortRequested = true;
    }

protected:
    void launch() override {
        QThreadPool::globalInstance()->start(this, m_priority);
    }

private:
    bool m_isAbortRequested = false;
    int m_priority = 0;
};

} // namespace qttask
