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

void initScriptEngine(QJSEngine* jsEngine, const ApplicationPtr& app);
void logScriptError(const QJSValue& jsVal, const char* functionName = nullptr);

} // namespace Mayo
