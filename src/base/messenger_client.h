/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

namespace Mayo {

class Messenger;

// Provides a link to a Messenger object
// The object returned by MessengerClient::messenger() is guaranteed to be non-nullptr
class MessengerClient {
public:
    MessengerClient();

    Messenger* messenger() const { return m_messenger; }
    void setMessenger(Messenger* messenger);

private:
    Messenger* m_messenger = nullptr;
};

} // namespace Mayo
