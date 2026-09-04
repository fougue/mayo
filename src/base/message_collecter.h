/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "messenger.h"

#include <gsl/span>
#include <vector>

namespace Mayo {

// Collects emitted messages into an array
class MessageCollecter : public Messenger {
public:
    void only(MessageType msgType);
    void ignore(MessageType msgType);
    bool isIgnored(MessageType msgType) const;

    void emitMessage(MessageType msgType, std::string_view text) override;

    gsl::span<const Messenger::Message> messages() const;
    std::string asString(std::string_view separator, MessageType msgType) const;
    std::string asString(std::string_view separator) const;

    unsigned messageCount(MessageType msgType) const;

    void clear();

private:
    static unsigned toFlag(MessageType msgType);

    unsigned m_ignoredTypes = 0;
    std::vector<Messenger::Message> m_vecMessage;
};

} // namespace Mayo
