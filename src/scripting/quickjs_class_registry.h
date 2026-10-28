/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <quickjs.h>

#include <string_view>
#include <unordered_map>

namespace Mayo {

// Associates QuickJS class names with class IDs registered in a runtime
// The registry does not own the runtime or the class-name strings. The runtime must outlive the
// registry, and every name passed to newClass() must remain valid for the lifetime of the registry
// Like the associated QuickJS runtime, the registry must only be accessed from one thread at a time
class QuickJsClassRegistry {
public:
    // Creates a registry for `runtime`. The runtime must remain alive while the registry is used
    QuickJsClassRegistry(JSRuntime* runtime);

    // Returns the non-owning QuickJS runtime associated with this registry
    JSRuntime* runtime() const { return m_runtime; }

    // Registers a QuickJS class with `name` and an optional finalizer
    // Returns false if the name is already registered or QuickJS cannot register the class
    // `name` must remain valid for the lifetime of this registry
    bool newClass(const char* name, JSClassFinalizer* finalizer = nullptr);

    // Returns the class ID associated with `name` or JS_INVALID_CLASS_ID if it is not registered
    JSClassID findClassId(const char* name) const;

private:
    JSRuntime* m_runtime{nullptr};
    std::unordered_map<std::string_view, JSClassID> m_nameToId;
};

} // namespace Mayo
