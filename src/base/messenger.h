/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <functional>
#include <string_view>

namespace Mayo {

// Provides a general-purpose interface to issue text messages without knowledge of how these
// messages will be further processed
// TODO Support stream-like syntax, eg:
//      messenger->info() << "Something happened, value: " << valueInt;
class Messenger {
public:
    enum class MessageType {
        Trace,
        Info,
        Warning,
        Error
    }; 

    // Dispatch the message 'text' to all observers
    virtual void emitMessage(MessageType msgType, std::string_view text) = 0;

    // Convenience functions around emitMessage()
    void emitTrace(std::string_view text);
    void emitInfo(std::string_view text);
    void emitWarning(std::string_view text);
    void emitError(std::string_view text);

    static Messenger& null();
};

// Provides facility to construct a Messenger object from a lambda
// This avoids to subclass Messenger
class MessengerByCallback : public Messenger {
public:
    MessengerByCallback(std::function<void(MessageType, std::string_view)> fnCallback);
    void emitMessage(MessageType msgType, std::string_view text) override;

private:
    std::function<void(MessageType, std::string_view)> m_fnCallback;
};

} // namespace Mayo
