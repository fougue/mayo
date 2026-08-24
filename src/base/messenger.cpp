/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "messenger.h"

#include <string>

namespace Mayo {

namespace {

class NullMessenger : public Messenger {
public:
    void emitMessage(MessageType, std::string_view) override {}
};

} // namespace

MessageStream::MessageStream(MessageType type, Messenger& messenger)
    : m_type(type), m_messenger(messenger)
{
}

//Message::Message(const Message& other)
//    : m_type(other.m_type), m_messenger(other.m_messenger), m_buffer(other.m_buffer)
//{
//}

MessageStream::~MessageStream()
{
    const std::string str = m_istream.str();
    switch (m_type) {
    case MessageType::Trace:
        m_messenger.emitTrace(str);
        break;
    case MessageType::Info:
        m_messenger.emitInfo(str);
        break;
    case MessageType::Warning:
        m_messenger.emitWarning(str);
        break;
    case MessageType::Error:
        m_messenger.emitError(str);
        break;
    }
}

MessageStream& MessageStream::space()
{
    m_istream << ' ';
    return *this;
}

MessageStream& MessageStream::operator<<(bool v)
{
    m_istream << (v ? "true" : "false");
    return *this;
}

void Messenger::emitTrace(std::string_view text)
{
    this->emitMessage(MessageType::Trace, text);
}

void Messenger::emitInfo(std::string_view text)
{
    this->emitMessage(MessageType::Info, text);
}

void Messenger::emitWarning(std::string_view text)
{
    this->emitMessage(MessageType::Warning, text);
}

void Messenger::emitError(std::string_view text)
{
    this->emitMessage(MessageType::Error, text);
}

MessageStream Messenger::trace()
{
    return MessageStream(MessageType::Trace, *this);
}

MessageStream Messenger::info()
{
    return MessageStream(MessageType::Info, *this);
}

MessageStream Messenger::warning()
{
    return MessageStream(MessageType::Warning, *this);
}

MessageStream Messenger::error()
{
    return MessageStream(MessageType::Error, *this);
}

Messenger& Messenger::null()
{
    static NullMessenger null;
    return null;
}

} // namespace Mayo
