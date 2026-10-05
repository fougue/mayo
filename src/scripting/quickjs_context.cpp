/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "quickjs_context.h"

namespace Mayo {

QuickJsValue QuickJsContext::eval(std::string_view script, const FilePath& filename, int evalFlags) const
{
    return {
        this->get(),
        JS_Eval(this->get(), script.data(), script.size(), filename.u8string().c_str(), evalFlags)
    };
}

QuickJsValue QuickJsContext::evalFunction(QuickJsValue funObj) const
{
    return { this->get(), JS_EvalFunction(this->get(), funObj.release()) };
}

QuickJsValue QuickJsContext::takeException() const
{
    return { this->get(), JS_GetException(this->get()) };
}

QuickJsValue QuickJsContext::takeException(JSContext* context)
{
    return { context, JS_GetException(context) };
}

QuickJsValue QuickJsContext::getModuleNamespace(JSModuleDef* module) const
{
    return { this->get(), JS_GetModuleNamespace(this->get(), module) };
}

} // namespace Mayo
