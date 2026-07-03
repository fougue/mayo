/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
