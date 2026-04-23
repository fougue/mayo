/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "messenger_client.h"
#include "messenger.h"

namespace Mayo {

MessengerClient::MessengerClient()
    : m_messenger(&Messenger::null())
{
}

void MessengerClient::setMessenger(Messenger* messenger)
{
    if (messenger)
        m_messenger = messenger;
    else
        m_messenger = &Messenger::null();
}

} // namespace Mayo
