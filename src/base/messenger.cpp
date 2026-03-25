/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "messenger.h"

#include <cassert>
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

MessengerByCallback::MessengerByCallback(std::function<void(MessageType, std::string_view)> fnCallback)
    : m_fnCallback(std::move(fnCallback))
{
}

void MessengerByCallback::emitMessage(MessageType msgType, std::string_view text)
{
    if (m_fnCallback)
        m_fnCallback(msgType, text);
}

unsigned MessageCollecter::toFlag(MessageType msgType)
{
    auto msgFlag = static_cast<unsigned>(msgType);
    assert(msgFlag < (8 * sizeof(unsigned) - 1));
    return 1 << msgFlag;
}

void MessageCollecter::only(MessageType msgType)
{
    m_ignoredTypes = ~(toFlag(msgType));
}

void MessageCollecter::ignore(MessageType msgType)
{
    m_ignoredTypes |= toFlag(msgType);
}

bool MessageCollecter::isIgnored(MessageType msgType) const
{
    return (m_ignoredTypes & toFlag(msgType)) != 0;
}

void MessageCollecter::emitMessage(MessageType msgType, std::string_view text)
{
    if (!this->isIgnored((msgType))) {
        const std::string stext{text};
        const Message msg{ msgType, std::move(stext) };
        m_vecMessage.push_back(std::move(msg));
    }
}

gsl::span<const Messenger::Message> MessageCollecter::messages() const
{
    return m_vecMessage;
}

std::string MessageCollecter::asString(std::string_view separator, MessageType msgType) const
{
    std::string str;
    for (const auto& msg : m_vecMessage) {
        if (msg.type == msgType) {
            str += msg.text;
            if (&msg != &m_vecMessage.back())
                str += separator;
        }
    }

    return str;
}

std::string MessageCollecter::asString(std::string_view separator) const
{
    std::string str;
    for (const auto& msg : m_vecMessage) {
        str += msg.text;
        if (&msg != &m_vecMessage.back())
            str += separator;
    }

    return str;
}

void MessageCollecter::clear()
{
    m_vecMessage.clear();
}

} // namespace Mayo
