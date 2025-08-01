/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include <TDF_Label.hxx>
#include <functional>
class QJSEngine;
class QJSValue;

namespace Mayo {

class PropertyValueConversion;
class TaskProgress;
namespace IO { class System; }
namespace IO { class ParametersProvider; }

// Structure used to pass rich set of parameters to initScriptEngine() function
struct ScriptEnvironment {
    const IO::System* ioSystem = nullptr;
    const IO::ParametersProvider* ioParametersProvider = nullptr; // TODO Rename to ioDefaultParametersProvider?
    std::function<void(TDF_Label, TaskProgress*)> ioEntityImportPostProcess;
    const PropertyValueConversion* propertyValueConverter = nullptr;
    static const PropertyValueConversion& getPropertyValueConverter(const ScriptEnvironment& env);
};

void initScriptEngine(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env);
void logScriptError(const QJSValue& jsVal, const char* functionName = nullptr);

} // namespace Mayo
