/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "quickjs_value.h"
#include "quickjs_binding_helper.h"

#include "../base/document_ptr.h"

namespace Mayo {

class QuickJsClassRegistry;

class QuickJsMayoDocument {
public:
    using Binding = QuickJsBindingHelper<QuickJsMayoDocument>;

    static constexpr const char* className = "MayoDocument";

    explicit QuickJsMayoDocument(DocumentPtr document);

    static QuickJsValue create(JSContext* context, DocumentPtr document);
    static QuickJsValue createPrototype(JSContext* context);

private:
    static JSValue name(JSContext* context, JSValueConst thisValue);
    static JSValue filePath(JSContext* context, JSValueConst thisValue);
    static JSValue entityCount(JSContext* context, JSValueConst thisValue);
    static DocumentPtr getDocumentOrThrow(JSContext* context, JSValueConst thisValue);

    DocumentPtr m_document;
};

} // namespace Mayo
