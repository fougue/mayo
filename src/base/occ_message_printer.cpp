/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "occ_message_printer.h"

#include "messenger.h"
#include "string_conv.h"

namespace Mayo {

namespace {

MessageType toMessageType(Message_Gravity gravity)
{
    switch (gravity) {
    case Message_Trace:
        return MessageType::Trace;
    case Message_Info:
        return MessageType::Info;
    case Message_Warning:
        return MessageType::Warning;
    case Message_Alarm:
        return MessageType::Warning;
    case Message_Fail:
        return MessageType::Error;
    }

    return MessageType::Trace;
}

} // namespace

OccMessagePrinter::OccMessagePrinter(Messenger* messenger)
    : m_messenger(messenger)
{
}

#if OCC_VERSION_HEX >= 0x070500
void OccMessagePrinter::send(const TCollection_AsciiString& text, const Message_Gravity gravity) const
{
    if (m_messenger)
        m_messenger->emitMessage(toMessageType(gravity), to_stdStringView(text));
}
#else
void OccMessagePrinter::Send(const TCollection_ExtendedString& text, const Message_Gravity gravity, const bool /*toPutEol*/) const
{
    if (m_messenger)
        m_messenger->emitMessage(toMessageType(gravity), to_stdString(text));
}
#endif

} // namespace Mayo
