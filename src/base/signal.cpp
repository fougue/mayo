/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "signal.h"

namespace Mayo {

std::unique_ptr<ISignalThreadHelper> globalHelper;

void setGlobalSignalThreadHelper(std::unique_ptr<ISignalThreadHelper> helper)
{
    globalHelper = std::move(helper);
}

ISignalThreadHelper* getGlobalSignalThreadHelper()
{
    return globalHelper.get();
}

ScopedSignalConnection::ScopedSignalConnection(const SignalConnectionHandle& hnd)
    : m_hnd(hnd)
{}

ScopedSignalConnection::~ScopedSignalConnection()
{
    m_hnd.disconnect();
}

void ScopedSignalConnection::set(const SignalConnectionHandle& hnd)
{
    m_hnd = hnd;
}

ScopedSignalConnection& ScopedSignalConnection::operator=(const SignalConnectionHandle& hnd)
{
    this->set(hnd);
    return *this;
}

} // namespace Mayo
