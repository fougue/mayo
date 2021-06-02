/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "messenger.h"

namespace Mayo {

void Messenger::emitTrace(const QString& text)
{
    this->emitMessage(MessageType::Trace, text);
}

void Messenger::emitInfo(const QString& text)
{
    this->emitMessage(MessageType::Info, text);
}

void Messenger::emitWarning(const QString& text)
{
    this->emitMessage(MessageType::Warning, text);
}

void Messenger::emitError(const QString& text)
{
    this->emitMessage(MessageType::Error, text);
}

MessengerQtSignal::MessengerQtSignal(QObject* parent)
    : QObject(parent)
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qRegisterMetaType<MessageType>("Messenger::MessageType");
        metaTypesRegistered = true;
    }
}

void MessengerQtSignal::emitMessage(MessageType msgType, const QString& text)
{
    emit this->message(msgType, text);
}

MessengerQtSignal* MessengerQtSignal::defaultInstance()
{
    static MessengerQtSignal messenger;
    return &messenger;
}

MessengerByCallback::MessengerByCallback(std::function<void(MessageType, QString)> fnCallback)
    : m_fnCallback(std::move(fnCallback))
{
}

void MessengerByCallback::emitMessage(Messenger::MessageType msgType, const QString& text)
{
    if (m_fnCallback)
        m_fnCallback(msgType, text);
}

void NullMessenger::emitMessage(Messenger::MessageType /*msgType*/, const QString& /*text*/)
{
}

Messenger* NullMessenger::instance()
{
    static NullMessenger nullMsg;
    return &nullMsg;
}

} // namespace Mayo
