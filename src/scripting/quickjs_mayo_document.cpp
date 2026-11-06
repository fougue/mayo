/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_mayo_document.h"

#include "quickjs_class_registry.h"
#include "quickjs_context.h"
#include "quickjs_script_engine.h"

#include "../base/document.h"

#include <memory>
#include <utility>

namespace Mayo {

QuickJsMayoDocument::QuickJsMayoDocument(DocumentPtr document)
    : m_document(std::move(document))
{}

QuickJsValue QuickJsMayoDocument::create(JSContext* context, DocumentPtr document)
{
    if (document.IsNull())
        return QuickJsContext::throwError(context, "Cannot create a Mayo.Document from a null Document");

    const JSClassID id = Binding::classId(context);
    if (id == JS_INVALID_CLASS_ID)
        return QuickJsContext::throwInternalError(context, "MayoDocument class is not registered");

    auto opaque = std::make_unique<QuickJsMayoDocument>(std::move(document));
    auto jsObject = QuickJsValue::newObjectClass(context, id);
    if (jsObject.isException())
        return jsObject;

    jsObject.setOpaquePtr(opaque.release());
    return jsObject;
}

JSValue QuickJsMayoDocument::name(JSContext* context, JSValueConst thisValue)
{
    auto doc = getDocumentOrThrow(context, thisValue);
    return doc
        ? QuickJsValue::newString(context, doc->name()).release()
        : JS_EXCEPTION;
}

JSValue QuickJsMayoDocument::filePath(JSContext* context, JSValueConst thisValue)
{
    auto doc = getDocumentOrThrow(context, thisValue);
    return doc
        ? QuickJsValue::newString(context, doc->filePath().u8string()).release()
        : JS_EXCEPTION;
}

JSValue QuickJsMayoDocument::entityCount(JSContext* context, JSValueConst thisValue)
{
    auto doc = getDocumentOrThrow(context, thisValue);
    return doc
        ? QuickJsValue::newInt32(context, doc->entityCount()).release()
        : JS_EXCEPTION;
}

QuickJsValue QuickJsMayoDocument::createPrototype(JSContext* context)
{
    auto prototype = QuickJsValue::newObject(context);
    if (prototype.isException())
        return prototype;

    const JSCFunctionListEntry prototypeProps[] = {
        quickjs_createPropertyGetter("name", &QuickJsMayoDocument::name),
        quickjs_createPropertyGetter("filePath", &QuickJsMayoDocument::filePath),
        quickjs_createPropertyGetter("entityCount", &QuickJsMayoDocument::entityCount)
    };
    if (JS_SetPropertyFunctionList(context, prototype.get(), prototypeProps, 3) < 0)
        return { context, JS_EXCEPTION };

    const JSClassID id = Binding::classId(context);
    if (id == JS_INVALID_CLASS_ID)
        return QuickJsContext::throwInternalError(context, "MayoDocument class is not registered");

    JS_SetClassProto(context, id, prototype.dup().release());
    return prototype;
}

DocumentPtr QuickJsMayoDocument::getDocumentOrThrow(JSContext* context, JSValue thisValue)
{
    auto opaque = Binding::getOpaque(context, thisValue);
    if (!opaque)
        return {}; // JS_GetOpaque2() already set exception

    if (opaque->m_document.IsNull()) {
        JS_ThrowInternalError(context, "Document is no longer available");
        return {};
    }

    return opaque->m_document;
}

} // namespace Mayo
