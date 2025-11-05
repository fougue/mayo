/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../qtscripting/script_engine.h"

#include <QtWidgets/QDialog>

class QTreeWidgetItem;

namespace Mayo {

class ScriptEngine;

// Provides a dialog to control execution of a JS script
class DialogExecScript : public QDialog {
    Q_OBJECT
public:
    DialogExecScript(ScriptEngine* engine, QWidget* parent = nullptr);
    ~DialogExecScript();

private:
    void onScriptEvaluateStarted();
    void onScriptEvaluateEnded(ScriptEngine::Result evalResult, ScriptEngine::EndReason endReason);

    void addOutputMessage(const ScriptEngine::Message& msg);

    void tryCloseDialog();

    void onOutputListItemClicked(QTreeWidgetItem* item);

    class Ui_DialogExecScript* m_ui = nullptr;
    ScriptEngine* m_scriptEngine = nullptr;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo
