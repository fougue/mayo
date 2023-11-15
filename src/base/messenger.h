/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <functional>
#include <sstream>
#include <string_view>

namespace Mayo {

class Messenger;

enum class MessageType {
    Trace,
    Info,
    Warning,
    Error
};

// Provides stream-like syntax support, eg:
//      messenger->info() << "Something happened, value: " << valueInt;
class MessageStream {
public:
    MessageStream(MessageType type, Messenger& messenger);
    MessageStream(const MessageStream&) = delete;
    MessageStream& operator=(const MessageStream&) = delete;
    ~MessageStream();

    MessageType messageType() const { return m_type; }
    std::istream& istream() { return m_istream; }

    MessageStream& space();
    MessageStream& operator<<(bool);

    template<typename T>
    MessageStream& operator<<(T t) {
        m_istream << t;
        return *this;
    }

private:
    MessageType m_type = MessageType::Trace;
    Messenger& m_messenger;
    std::stringstream m_istream;
};

// Provides a general-purpose interface to issue text messages without knowledge of how these
// messages will be further processed
class Messenger {
public:
    // Dispatch the message 'text' to all observers
    virtual void emitMessage(MessageType msgType, std::string_view text) = 0;

    // Convenience functions around emitMessage()
    void emitTrace(std::string_view text);
    void emitInfo(std::string_view text);
    void emitWarning(std::string_view text);
    void emitError(std::string_view text);

    MessageStream trace();
    MessageStream info();
    MessageStream warning();
    MessageStream error();

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
