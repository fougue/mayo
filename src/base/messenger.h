/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

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
    struct Message {
        MessageType type;
        std::string text;
    };

    virtual ~Messenger() = default;

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

} // namespace Mayo
