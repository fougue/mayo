/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "../base/filepath.h"
#include "../base/task_manager.h"

#include <QtWidgets/QDialog>
#include <functional>

class QJSEngine;

namespace Mayo {

// Provides a dialog to control execution of a JS script
class DialogExecScript : public QDialog {
    Q_OBJECT
public:
    // Ctor & dtor
    DialogExecScript(QWidget* parent = nullptr);
    ~DialogExecScript();

    // Set the function used to create a JS engine to evaluate scripts
    using ScriptEngineCreator = std::function<QJSEngine*(QObject*)>;
    void setScriptEngineCreator(ScriptEngineCreator fn);

    // Path to the script file to be executed
    void setScriptFilePath(const FilePath& scriptFilePath);

    // Asynchronous execution of script file defined with setScriptFilePath()
    // Does nothing(returns) if script execution is currently running
    void startScript();

private:
    void onTaskStarted(TaskId taskId);
    void onTaskEnded(TaskId taskId);

    void interruptScriptExec();
    void restartOrStopScriptExec();
    void recreateScriptEngine();

    struct Message {
        QtMsgType type = QtDebugMsg;
        QString text;
        QString contextFile;
        int contextLine = -1;
    };

    void addConsoleOutput(const Message& msg);

    void tryCloseDialog();

    class Ui_DialogExecScript* m_ui = nullptr;
    ScriptEngineCreator m_fnScriptEngineCreator;
    QJSEngine* m_jsEngine = nullptr;
    QString m_scriptFilePath;
    TaskManager m_taskMgr;
    TaskId m_scriptExecTaskId = TaskId_null;
    bool m_wasScriptExecInterrupted = false;
    bool m_scriptExecIsRunning = false;
};

} // namespace Mayo
