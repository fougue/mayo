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

namespace qttask {

struct CurrentThread { };

/*! \brief Task runner executing in the current thread context
 */
template<>
class Runner<CurrentThread> : public BaseRunner
{
public:
    Runner<CurrentThread>(const Manager *mgr)
        : BaseRunner(mgr)
    { }

protected:
    bool isAbortRequested() override
    { return m_isAbortRequested; }

    void requestAbort() override
    { m_isAbortRequested = true; }

    void launch() override
    { this->execRunnableFunc(); }

private:
    bool m_isAbortRequested = false;
};

} // namespace qttask
