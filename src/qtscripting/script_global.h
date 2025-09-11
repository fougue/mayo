/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"
#include <TDF_Label.hxx>
#include <functional>
#include <string_view>
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

// Configures the JS engine so it can support Mayo Scripting API
//
// * Installs the required JS extensions(eg Console)
//
// * Registers a global 'application' JS object bound to 'app' parameter. This implies JS scripts
//   can access the already loaded documents in Mayo
//
// * Registers Mayo Scripting enumerations as plain objects for cleaner syntax in JS scripts
//   Example:
//       GeomCurveType {
//           Line: intLiteral,
//           Circle: intLiteral,
//           Ellipse: intLiteral,
//           ...
//        };
//   To be used in JS code as GeomCurveType.Circle instead of error-prone int/string literal
//   This also includes enums declared for any IO reader/writer provided by the IO::System object
//   of input ScriptEnvironment
void initScriptEngine(QJSEngine* jsEngine, const ApplicationPtr& app, const ScriptEnvironment& env);

// Generates a critical message to the logging framework if the JS value owns an error(QJSValue::isError())
void logScriptError(const QJSValue& jsVal, const char* functionName = nullptr);

// Overload of the above function, but an error is created using the provided JS engine
void logScriptError(QJSEngine* jsEngine, std::string_view message, const char* functionName = nullptr);

} // namespace Mayo
