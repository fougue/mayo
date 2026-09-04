/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "message_collecter.h"

#include <algorithm>
#include <cassert>
#include <string>

namespace Mayo {

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

unsigned MessageCollecter::messageCount(MessageType msgType) const
{
    return std::count_if(
        m_vecMessage.cbegin(),
        m_vecMessage.cend(),
        [](const Message& msg) { return msg.type == MessageType::Error; }
    );
}

void MessageCollecter::clear()
{
    m_vecMessage.clear();
}

} // namespace Mayo
