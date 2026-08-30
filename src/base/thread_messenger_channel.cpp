/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "thread_messenger_channel.h"

#include "occ_message_printer.h"

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <mutex>

namespace Mayo {

namespace {

Messenger*& ThreadMessengerChannel_current()
{
    static thread_local Messenger* ptr = nullptr;
    return ptr;
}

void ThreadMessengerChannel_setCurrent(Messenger* ptr)
{
    ThreadMessengerChannel_current() = ptr;
}

} // namespace

void ThreadMessengerChannel::emitMessage(MessageType type, std::string_view text)
{
    Messenger* targetMessenger = ThreadMessengerChannel_current();
    if (targetMessenger)
        targetMessenger->emitMessage(type, text);
}

ThreadMessengerChannel::Scope::Scope(Messenger* messenger)
    : m_prevMessenger(ThreadMessengerChannel_current())
{
    ThreadMessengerChannel_setCurrent(messenger);
}

ThreadMessengerChannel::Scope::~Scope()
{
    ThreadMessengerChannel_setCurrent(m_prevMessenger);
}

const OccHandle<Message_Printer>& ThreadMessengerChannel::addGlobalOccPrinter()
{
    static std::once_flag onceFlag;
    static OccHandle<Message_Printer> installedPrinter;
    static ThreadMessengerChannel channel;

    std::call_once(onceFlag, [&]{
        installedPrinter = makeOccHandle<OccMessagePrinter>(&channel);
        ::Message::DefaultMessenger()->AddPrinter(installedPrinter);
    });
    return installedPrinter;
}

} // namespace Mayo
