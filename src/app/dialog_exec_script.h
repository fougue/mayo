/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../qtscripting/script_engine.h"

#include <QtWidgets/QDialog>

#include <functional>

class QFileSystemWatcher;
class QLineEdit;
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

    struct TextFilter {
        enum Option {
            None = 0x0,
            UseRegExp = 0x01,
            CaseSensitive = 0x02,
            All = 0xFF
        };
        using Options = unsigned;

        QString key;
        Options options = Option::None;
    };

    void applyOutputListFilter(const TextFilter& filter);
    void applyOutputTextFilter(const TextFilter& filter);

    using ApplyTextFilter = std::function<void(const TextFilter&)>;
    static void installFilterLineEdit(
        QLineEdit* lineEdit, TextFilter::Options options, ApplyTextFilter fnApplyFilter
    );

    class Ui_DialogExecScript* m_ui = nullptr;
    ScriptEngine* m_scriptEngine = nullptr;
    QFileSystemWatcher* m_fileSystemWatcher = nullptr;
    ScopedSignalConnections<> m_sigConns;
};

} // namespace Mayo
