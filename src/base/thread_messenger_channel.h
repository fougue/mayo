/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "messenger.h"
#include "occ_handle.h"

#include <Message_Printer.hxx>

namespace Mayo {

// Messenger that forwards to whichever Messenger is active for the calling thread (or discards
// silently if none is set)
// Bridges single-instance callback APIs (eg OpenCascade's DefaultMessenger()) with per-operation
// Messenger routing in Mayo. Register one instance with the external source, then bind per-operation
// Routing is thread-local; nested Scope on the same thread is supported
class ThreadMessengerChannel : public Messenger {
public:
    ThreadMessengerChannel() = default;

    // Forwards `text` to the active Messenger for this thread, if any
    void emitMessage(MessageType type, std::string_view text) override;

    // RAII: binds `messenger` as active for the calling thread, restores previous on destruction
    class Scope {
    public:
        explicit Scope(Messenger* messenger);
        ~Scope();
        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
    private:
        Messenger* m_prevMessenger = nullptr;
    };

    // Registers a Message_Printer forwarding OpenCascade messages to a global ThreadMessengerChannel
    // instance. Safe to call multiple times/threads: installed exactly once, on the first call
    static const OccHandle<Message_Printer>& addGlobalOccPrinter();
};

} // namespace Mayo
