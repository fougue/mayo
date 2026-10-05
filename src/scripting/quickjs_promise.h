/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"

#include <string_view>

namespace Mayo {

// C++ wrapper around a JavaScript Promise
//
// Owns the Promise and its resolving and rejecting functions
// All operations must be performed from the thread owning the QuickJS context
class QuickJsPromise {
public:
    // Creates a new JS Promise and its resolving functions
    explicit QuickJsPromise(JSContext* context);

    // Releases all JS values owned by this object
    // Must be called from the thread owning the QuickJS context
    ~QuickJsPromise() = default;

    QuickJsPromise(QuickJsPromise&&) noexcept = default;
    QuickJsPromise& operator=(QuickJsPromise&&) noexcept = default;

    // Not copyable
    QuickJsPromise(const QuickJsPromise&) = delete;
    QuickJsPromise& operator=(const QuickJsPromise&) = delete;

    // Returns a duplicated reference to the JS Promise
    QuickJsValue get() const;

    JSPromiseStateEnum state() const;

    // Returns the result value of the promise
    // The result is the fulfilled value when the promise is fulfilled, or the rejection reason when
    // the promise is rejected
    // The promise should be settled before calling this function
    QuickJsValue result() const;

    // Resolves the Promise with the given JS value
    bool resolve(JSValueConst value);

    // Rejects the Promise with the given JS value
    bool reject(JSValueConst reason);

    // Rejects the Promise with a string reason
    bool reject(std::string_view strMessage);

private:
    JSContext* m_context = nullptr;
    QuickJsValue m_promise;
    QuickJsValue m_resolve;
    QuickJsValue m_reject;
};

} // namespace Mayo
