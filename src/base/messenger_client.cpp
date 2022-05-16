/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
