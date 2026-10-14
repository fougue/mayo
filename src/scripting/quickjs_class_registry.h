/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include <quickjs.h>

#include <string_view>
#include <unordered_map>

namespace Mayo {

class QuickJsClassRegistry {
public:
    QuickJsClassRegistry(JSRuntime* runtime);

    JSRuntime* runtime() const { return m_runtime; }

    bool newClass(const char* name, JSClassFinalizer* finalizer = nullptr);
    JSClassID findClassId(const char* name) const;

private:
    JSRuntime* m_runtime{nullptr};
    std::unordered_map<std::string_view, JSClassID> m_nameToId;
};

} // namespace Mayo
