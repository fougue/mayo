/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/application_ptr.h"

class QJSEngine;
class QJSValue;
class QObject;

namespace Mayo {

class Application;

QJSEngine* createScriptEngine(const ApplicationPtr& app, QObject* parent = nullptr);
void logScriptError(const QJSValue& jsVal, const char* functionName = nullptr);

} // namespace Mayo
