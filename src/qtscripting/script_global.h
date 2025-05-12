/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/application_ptr.h"

class QJSEngine;
class QJSValue;
class QObject;

namespace Mayo {

class Application;
namespace IO { class System; }

// TODO Replace arguments with ScriptEnvironnment class that will provide:
//         ApplicationPtr
//         const IO::System*
//         const IO::ParametersProvider*
//         EntityPostProcess function(meshing of BRep shapes)
//         ...
void initScriptEngine(QJSEngine* jsEngine, const ApplicationPtr& app, const IO::System* ioSystem);
void logScriptError(const QJSValue& jsVal, const char* functionName = nullptr);

} // namespace Mayo
