/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"
#include "quickjs_binding_helper.h"

#include "../base/document_ptr.h"

namespace Mayo {

class IThreadDispatch;
class QuickJsClassRegistry;

// QuickJS binding for a Mayo Document
// The wrapper owns a DocumentPtr and optionally dispatches document access to the owning application's thread
class QuickJsMayoDocument {
public:
    using Binding = QuickJsBindingHelper<QuickJsMayoDocument>;

    static constexpr const char* className = "MayoDocument";

    QuickJsMayoDocument(DocumentPtr document, IThreadDispatch* threadDispatch);

    static QuickJsValue create(
        JSContext* context, DocumentPtr document, IThreadDispatch* threadDispatch = nullptr
    );
    static QuickJsValue createPrototype(JSContext* context);

private:
    static JSValue name(JSContext* context, JSValueConst thisValue);
    static JSValue filePath(JSContext* context, JSValueConst thisValue);
    static JSValue entityCount(JSContext* context, JSValueConst thisValue);

    static QuickJsMayoDocument* getOpaqueOrThrow(JSContext* context, JSValueConst thisValue);

    IThreadDispatch* m_threadDispatch{nullptr}; // Non-owning
    DocumentPtr m_document;
};

} // namespace Mayo
