/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "../src/base/signal.h"

#include <cstdint>
#include <type_traits>
#include <variant>
#include <vector>

namespace Mayo {

// Equivalent of QSignalSpy for KDBindings signals
struct SignalEmitSpy {
    struct UnknownType {};
    using ArgValue = std::variant<UnknownType, std::int64_t, std::uint64_t>;
    using SignalArguments = std::vector<ArgValue>;

    template<typename... Args>
    SignalEmitSpy(Signal<Args...>* signal)
    {
        this->sigConnection = signal->connect([=](Args... args) {
            ++this->count;
            SignalArguments sigArgs;
            SignalEmitSpy::recordArgs(&sigArgs, args...);
            this->vecSignals.push_back(std::move(sigArgs));
        });
    }

    ~SignalEmitSpy()
    {
        this->sigConnection.disconnect();
    }

    static void recordArgs(SignalArguments* /*ptr*/)
    {
    }

    template<typename Arg, typename... Args>
    static void recordArgs(SignalArguments* ptr, Arg arg, Args... args)
    {
        if constexpr (std::is_integral_v<Arg>) {
            if constexpr (std::is_signed_v<Arg>) {
                ptr->push_back(static_cast<std::int64_t>(arg));
            }
            else {
                ptr->push_back(static_cast<std::uint64_t>(arg));
            }
        }
        else {
            ptr->push_back(UnknownType{});
        }

        SignalEmitSpy::recordArgs(ptr, args...);
    }

    int count = 0;
    std::vector<SignalArguments> vecSignals;
    SignalConnectionHandle sigConnection;
};

} // namespace Mayo
