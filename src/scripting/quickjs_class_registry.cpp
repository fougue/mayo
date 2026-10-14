/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_class_registry.h"

namespace Mayo {

QuickJsClassRegistry::QuickJsClassRegistry(JSRuntime* runtime)
    : m_runtime(runtime)
{
}

bool QuickJsClassRegistry::newClass(const char* name, JSClassFinalizer* finalizer)
{
    if (m_nameToId.find(name) != m_nameToId.cend())
        return false; // Class is already registered

    JSClassID classId = JS_INVALID_CLASS_ID;
    JS_NewClassID(m_runtime, &classId);
    if (classId == JS_INVALID_CLASS_ID)
        return false;

    JSClassDef classDef{};
    classDef.class_name = name;
    classDef.finalizer = finalizer;
    if (JS_NewClass(m_runtime, classId, &classDef) == 0) {
        m_nameToId.emplace(name, classId);
        return true;
    }

    return false;
}

JSClassID QuickJsClassRegistry::findClassId(const char* name) const
{
    auto it = m_nameToId.find(name);
    return it != m_nameToId.cend() ? it->second : JS_INVALID_CLASS_ID;
}

} // namespace Mayo

