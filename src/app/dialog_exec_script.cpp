/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "dialog_exec_script.h"

#include "qtgui_utils.h"
#include "qtwidgets_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qtcore_utils.h"
#include "../qtcommon/qstring_conv.h"
#include "ui_dialog_exec_script.h"

#include <QtCore/QFileInfo>

namespace Mayo {

DialogExecScript::DialogExecScript(ScriptEngine* engine, QWidget* parent)
    : QDialog(parent),
      m_ui(new Ui_DialogExecScript),
      m_scriptEngine(engine)
{
    m_ui->setupUi(this);
    m_sigConns
        << m_scriptEngine->signalMessage.connectSlot(&DialogExecScript::addConsoleOutput, this)
        << m_scriptEngine->signalEvaluateStarted.connectSlot(&DialogExecScript::onScriptEvaluateStarted, this)
        << m_scriptEngine->signalEvaluateEnded.connectSlot(&DialogExecScript::onScriptEvaluateEnded, this)
        ;
    QObject::connect(
        m_ui->btn_restartStop, &QAbstractButton::clicked,
        this, [=]{ m_scriptEngine->runOrStopEvaluate(); }
    );
    QObject::connect(
        m_ui->buttonBox->button(QDialogButtonBox::Close), &QAbstractButton::clicked,
        this, &DialogExecScript::tryCloseDialog
    );
}

DialogExecScript::~DialogExecScript()
{
    delete m_ui;
}

void DialogExecScript::onScriptEvaluateStarted()
{
    const QString strScriptFilePath = filepathTo<QString>(m_scriptEngine->scriptFilePath());
    m_ui->label_Status->setText(tr("Executing '%1'...").arg(strScriptFilePath));
    m_ui->btn_restartStop->setText(tr("Stop"));
    m_ui->progressBar_Execution->setRange(0, 0);
    m_ui->progressBar_Execution->setValue(-1);
    m_ui->treeWidget_Output->clear();
}

void DialogExecScript::onScriptEvaluateEnded(ScriptEngine::EndReason reason)
{
    const QString strScriptFilePath = filepathTo<QString>(m_scriptEngine->scriptFilePath());
    const bool wasEvaluateStopped = reason == ScriptEngine::EndReason::Stopped;
    if (!wasEvaluateStopped)
        m_ui->label_Status->setText(tr("Finished '%1'").arg(strScriptFilePath));
    else
        m_ui->label_Status->setText(tr("Stopped '%1'").arg(strScriptFilePath));

    m_ui->btn_restartStop->setText(tr("Restart"));
    m_ui->progressBar_Execution->setRange(0, 100);
    m_ui->progressBar_Execution->setValue(!wasEvaluateStopped ? 100 : 0);
    for (int col = 0; col < m_ui->treeWidget_Output->columnCount(); ++col)
        m_ui->treeWidget_Output->resizeColumnToContents(col);
}

void DialogExecScript::addConsoleOutput(const ScriptEngine::Message& msg)
{
    auto fnStrMsgType = [](MessageType type) -> QString {
        switch (type) {
        case MessageType::Trace: return tr("debug");
        case MessageType::Info: return tr("info");
        case MessageType::Warning: return tr("warning");
        case MessageType::Error: return tr("critical");
        default: return tr("?");
        }
    };

    auto item = new QTreeWidgetItem;
    item->setText(0, fnStrMsgType(msg.type));
    item->setFont(0, QtGuiUtils::FontChange(item->font(0)).scalePointSizeF(0.8).capitalization(QFont::AllUppercase));
    if (msg.type == MessageType::Error) {
        item->setBackground(0, QColor(Qt::red));
        item->setFont(0, QtGuiUtils::FontChange(item->font(0)).bold(true));
    }

    item->setText(1, to_QString(msg.text));
    item->setText(2, QFileInfo(to_QString(msg.contextFile)).fileName());
    item->setText(3, QString::number(msg.contextLine));
    m_ui->treeWidget_Output->addTopLevelItem(item);
}

void DialogExecScript::tryCloseDialog()
{
    if (m_scriptEngine->isEvaluateRunning()) {
        auto msgBox = QtWidgetsUtils::asyncMsgBoxWarning(
            this,
            tr("Warning"),
            tr("Script execution isn't finished\n\nInterrupt and exit?"),
            QMessageBox::Yes | QMessageBox::No
        );
        QObject::connect(msgBox, &QMessageBox::buttonClicked, this, [=](QAbstractButton* btn) {
            if (btn == msgBox->button(QMessageBox::Yes)) {
                m_scriptEngine->stopEvaluate();
                m_scriptEngine->waitForEvaluateEnd(10000/*ms*/);
                QtCoreUtils::runJobOnMainThread([=]{ this->tryCloseDialog(); });
            }
        });
    }
    else {
        this->reject();
    }
}

} // namespace Mayo
