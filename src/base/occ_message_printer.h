/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <Message_Printer.hxx>
#include <Standard_Version.hxx>

namespace Mayo {

class Messenger;

class OccMessagePrinter : public Message_Printer {
public:
    explicit OccMessagePrinter(Messenger* messenger);

#if OCC_VERSION_HEX >= 0x070500
protected:
    void send(const TCollection_AsciiString& text, const Message_Gravity gravity) const override;
#else
public:
    void Send(const TCollection_ExtendedString& text, const Message_Gravity gravity, const bool toPutEol) const override;
#endif

private:
    Messenger* m_messenger = nullptr;
};

DEFINE_STANDARD_HANDLE(OccMessagePrinter, Message_Printer)

} // namespace Mayo
