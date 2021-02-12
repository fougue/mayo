/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "messenger.h"

namespace Mayo {

Messenger::Messenger(QObject* parent)
    : QObject(parent)
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qRegisterMetaType<MessageType>("Messenger::MessageType");
        metaTypesRegistered = true;
    }
}

Messenger* Messenger::defaultInstance()
{
    static Messenger messenger;
    return &messenger;
}

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

void Messenger::emitMessage(MessageType msgType, const QString& text)
{
    emit this->message(msgType, text);
}

NullMessenger* NullMessenger::instance()
{
    static NullMessenger nullMsg;
    return &nullMsg;
}

void NullMessenger::emitMessage(Messenger::MessageType /*msgType*/, const QString& /*text*/)
{
}

} // namespace Mayo
