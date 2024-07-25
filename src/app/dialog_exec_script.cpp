/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_exec_script.h"

#include "qtwidgets_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/log_message_handler.h"
#include "../qtcommon/qtcore_utils.h"
#include "../qtscripting/script_global.h"
#include "ui_dialog_exec_script.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtQml/QJSEngine>
#include <gsl/util>

namespace Mayo {

namespace {

QString scriptProgram(const QString& strFilepath)
{
    QFile file(strFilepath);
    if (file.open(QIODevice::ReadOnly))
        return file.readAll();

    return {};
}

} // namespace

DialogExecScript::DialogExecScript(QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExecScript)
{
    m_ui->setupUi(this);
    // TODO Don't forget to disconnect those slots in destructor(maybe not needed as m_taskMgr will be out of scope)
    m_taskMgr.signalStarted.connectSlot(&DialogExecScript::onTaskStarted, this);
    m_taskMgr.signalEnded.connectSlot(&DialogExecScript::onTaskEnded, this);
    QObject::connect(
        m_ui->btn_restartStop, &QAbstractButton::clicked,
        this, &DialogExecScript::restartOrStopScriptExec
    );
    QObject::connect(
        m_ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked,
        this, &DialogExecScript::tryCloseDialog
    );
}

DialogExecScript::~DialogExecScript()
{
    delete m_ui;
    if (m_jsEngine)
        m_jsEngine->deleteLater();
}

void DialogExecScript::setScriptEngineCreator(ScriptEngineCreator fn)
{
    m_fnScriptEngineCreator = std::move(fn);
}

void DialogExecScript::setScriptFilePath(const FilePath& scriptFilePath)
{
    m_scriptFilePath = QDir::toNativeSeparators(filepathTo<QString>(scriptFilePath));
}

void DialogExecScript::startScript()
{
    if (m_scriptExecIsRunning)
        return;

    m_scriptExecTaskId = m_taskMgr.newTask([=](TaskProgress*) {
        // Override the "console output" handler of LogMessageHandler, first keep the current handler
        // so it can be restored before exiting
        auto& logMsgHandler = LogMessageHandler::instance();
        auto onEntryJsConsoleOutputHandler = logMsgHandler.jsConsoleOutputHandler();
        auto _ = gsl::finally([&]{
            logMsgHandler.setJsConsoleOutputHandler(onEntryJsConsoleOutputHandler);
        });
        auto fnAddConsoleOutput = [=](QtMsgType type, const QString& text, const QString& file, int line) {
            const Message msg = { type, text, file, line };
            QtCoreUtils::runJobOnMainThread([=]{ this->addConsoleOutput(msg); });
        };
        logMsgHandler.setJsConsoleOutputHandler(
            [=](QtMsgType type, const QMessageLogContext& context, const QString& text) {
                const QString strFile = context.file ? context.file : "";
                fnAddConsoleOutput(type, text, strFile, context.line);
            }
        );

        // Evaluate script program
        this->recreateScriptEngine();
        auto jsVal = m_jsEngine->evaluate(scriptProgram(m_scriptFilePath), m_scriptFilePath);
        logScriptError(jsVal);
    });
    m_taskMgr.run(m_scriptExecTaskId);
}

void DialogExecScript::onTaskStarted(TaskId taskId)
{
    if (m_scriptExecTaskId != taskId)
        return;

    m_scriptExecIsRunning = true;
    m_wasScriptExecInterrupted = false;
    m_ui->label_Status->setText(tr("Executing '%1'...").arg(m_scriptFilePath));
    m_ui->btn_restartStop->setText(tr("Stop"));
    m_ui->progressBar_Execution->setRange(0, 0);
    m_ui->progressBar_Execution->setValue(-1);
    m_ui->treeWidget_Output->clear();
}

void DialogExecScript::onTaskEnded(TaskId taskId)
{
    if (m_scriptExecTaskId != taskId)
        return;

    m_scriptExecIsRunning = false;
    if (!m_wasScriptExecInterrupted)
        m_ui->label_Status->setText(tr("Finished '%1'").arg(m_scriptFilePath));
    else
        m_ui->label_Status->setText(tr("Stopped '%1'").arg(m_scriptFilePath));

    m_ui->btn_restartStop->setText(tr("Restart"));
    m_ui->progressBar_Execution->setRange(0, 100);
    m_ui->progressBar_Execution->setValue(!m_wasScriptExecInterrupted ? 100 : 0);
    m_jsEngine->setInterrupted(false);

    for (int col = 0; col < m_ui->treeWidget_Output->columnCount(); ++col)
        m_ui->treeWidget_Output->resizeColumnToContents(col);
}

void DialogExecScript::interruptScriptExec()
{
    m_wasScriptExecInterrupted = true;
    m_jsEngine->setInterrupted(true);
}

void DialogExecScript::restartOrStopScriptExec()
{
    if (m_scriptExecIsRunning)
        this->interruptScriptExec();
    else
        this->startScript();
}

void DialogExecScript::recreateScriptEngine()
{
    delete m_jsEngine;
    m_jsEngine = m_fnScriptEngineCreator(nullptr);
}

void DialogExecScript::addConsoleOutput(const Message& msg)
{
    auto fnStrMsgType = [](QtMsgType type) -> QString {
        switch (type) {
        case QtDebugMsg: return tr("debug");
        case QtWarningMsg: return tr("warning");
        case QtCriticalMsg: return tr("critical");
        case QtFatalMsg: return tr("fatal");
        case QtInfoMsg: return tr("info");
        default: return tr("?");
        }
    };

    auto item = new QTreeWidgetItem;
    item->setText(0, fnStrMsgType(msg.type));
    item->setText(1, msg.text);
    item->setText(2, QFileInfo(msg.contextFile).fileName());
    item->setText(3, QString::number(msg.contextLine));
    m_ui->treeWidget_Output->addTopLevelItem(item);
}

void DialogExecScript::tryCloseDialog()
{
    if (m_scriptExecIsRunning) {
        auto msgBox = QtWidgetsUtils::asyncMsgBoxWarning(
            this,
            tr("Warning"),
            tr("Script execution isn't finished\n\nInterrupt and exit?"),
            QMessageBox::Yes | QMessageBox::No
        );
        QObject::connect(msgBox, &QMessageBox::buttonClicked, this, [=](QAbstractButton* btn) {
            if (btn == msgBox->button(QMessageBox::Yes)) {
                this->interruptScriptExec();
                m_taskMgr.waitForDone(m_scriptExecTaskId, 10000/*ms*/);
                QtCoreUtils::runJobOnMainThread([=]{ this->tryCloseDialog(); });
            }
        });
    }
    else {
        this->reject();
    }
}

} // namespace Mayo
