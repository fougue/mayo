/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../qtscripting/script_engine.h"

#include <QtGui/QTextCharFormat>
#include <QtWidgets/QDialog>

#include <vector>

class QAction;
class QFileSystemWatcher;
class QModelIndex;

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

    void onOutputListItemClicked(const QModelIndex& index);
    void onFileChanged(const QString& path);

    class OutputListModel;
    OutputListModel* outputListModel() const;

    enum TextFilterOption : unsigned {
        UseRegExp = 0x01,
        CaseSensitive = 0x02,
        IncludeDebugMessages = 0x10,
        IncludeInfoMessages = 0x20,
        IncludeWarningMessages = 0x40,
        IncludeErrorMessages = 0x80
    };
    using TextFilterOptions = unsigned;
    static constexpr TextFilterOptions TextFilterAllOptions = 0xFF;

    QAction* findAction(TextFilterOption option) const;
    bool isActionChecked(TextFilterOption option) const;

    void applyOutputFilter();

    void installOutputFilterLineEdit();

    class Ui_DialogExecScript* m_ui = nullptr;
    std::vector<std::pair<TextFilterOption, QAction*>> m_outputFilterActions;
    ScriptEngine* m_scriptEngine = nullptr;
    QFileSystemWatcher* m_fileSystemWatcher = nullptr;
    ScopedSignalConnections<> m_sigConns;
    QTextCharFormat m_defaultTextCharFormat;
};

} // namespace Mayo
