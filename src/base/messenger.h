/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <functional>
#include <string_view>

namespace Mayo {

class Messenger {
public:
    enum class MessageType {
        Trace,
        Info,
        Warning,
        Error
    };

    void emitTrace(std::string_view text);
    void emitInfo(std::string_view text);
    void emitWarning(std::string_view text);
    void emitError(std::string_view text);
    virtual void emitMessage(MessageType msgType, std::string_view text) = 0;

    static Messenger& null();
};

// Provides facility to construct a Messenger object from a lambda
// This avoids to subclass Messenger
class MessengerByCallback : public Messenger {
public:
    MessengerByCallback(std::function<void(MessageType, std::string_view)> fnCallback);
    void emitMessage(MessageType msgType, std::string_view text) override;

private:
    std::function<void(MessageType, std::string_view)> m_fnCallback;
};

} // namespace Mayo
