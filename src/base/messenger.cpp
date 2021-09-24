/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "messenger.h"

namespace Mayo {

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

MessengerByCallback::MessengerByCallback(std::function<void(MessageType, std::string_view)> fnCallback)
    : m_fnCallback(std::move(fnCallback))
{
}

void MessengerByCallback::emitMessage(Messenger::MessageType msgType, std::string_view text)
{
    if (m_fnCallback)
        m_fnCallback(msgType, text);
}

void NullMessenger::emitMessage(Messenger::MessageType /*msgType*/, std::string_view /*text*/)
{
}

Messenger* NullMessenger::instance()
{
    static NullMessenger nullMsg;
    return &nullMsg;
}

} // namespace Mayo
