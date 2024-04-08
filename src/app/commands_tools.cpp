/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_tools.h"

#include "../base/application.h"
#include "../base/application_item_selection_model.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "../qtcommon/filepath_conv.h"
#include "app_module.h"
#include "dialog_inspect_xde.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "qtwidgets_utils.h"
#include "theme.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QMenu>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QtDebug>
#include <QtQml/QJSEngine>
#include <QtWidgets/QFileDialog>

namespace Mayo {

CommandSaveViewImage::CommandSaveViewImage(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Save View to Image"));
    action->setToolTip(Command::tr("Save View to Image"));
    action->setIcon(mayoTheme()->icon(Theme::Icon::Camera));
    this->setAction(action);
}

void CommandSaveViewImage::execute()
{
    auto guiDoc = this->currentGuiDocument();
    auto dlg = new DialogSaveImageView(guiDoc->v3dView(), this->widgetMain());
    QtWidgetsUtils::asyncDialogExec(dlg);
}

bool CommandSaveViewImage::getEnabledStatus() const
{
    return this->app()->documentCount() != 0
           && this->context()->currentPage() == IAppContext::Page::Documents;
}

CommandInspectXde::CommandInspectXde(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Inspect XDE"));
    action->setToolTip(Command::tr("Inspect XDE"));
    this->setAction(action);
}

void CommandInspectXde::execute()
{
    const Span<const ApplicationItem> spanAppItem = this->guiApp()->selectionModel()->selectedItems();
    DocumentPtr doc;
    for (const ApplicationItem& appItem : spanAppItem) {
        if (appItem.document()->isXCafDocument()) {
            doc = appItem.document();
            break;
        }
    }

    if (doc) {
        auto dlg = new DialogInspectXde(this->widgetMain());
        dlg->load(doc);
        QtWidgetsUtils::asyncDialogExec(dlg);
    }
}

bool CommandInspectXde::getEnabledStatus() const
{
    Span<const ApplicationItem> spanSelectedAppItem = this->guiApp()->selectionModel()->selectedItems();
    const ApplicationItem firstAppItem =
            !spanSelectedAppItem.empty() ? spanSelectedAppItem.front() : ApplicationItem();
    return spanSelectedAppItem.size() == 1
            && firstAppItem.isValid()
            && firstAppItem.document()->isXCafDocument()
            && this->context()->currentPage() == IAppContext::Page::Documents
        ;
}

CommandEditOptions::CommandEditOptions(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Options"));
    action->setToolTip(Command::tr("Options"));
    this->setAction(action);
}

void CommandEditOptions::execute()
{
    auto dlg = new DialogOptions(AppModule::get()->settings(), this->widgetMain());
    QtWidgetsUtils::asyncDialogExec(dlg);
}

namespace {

void evaluateScript(QJSEngine* jsEngine, const FilePath& filepathScript)
{
    if (!jsEngine)
        return;

    auto fnJsEvaluate = [](QJSEngine* jsEngine, const QString& program) {
        auto jsVal = jsEngine->evaluate(program);
        if (jsVal.isError()) {
            qCritical() << "Error at line"
                        << jsVal.property("lineNumber").toInt()
                        << ":" << jsVal.toString();
        }

        return jsVal;
    };

    QFile jsFile(filepathTo<QString>(filepathScript));
    if (jsFile.open(QIODevice::ReadOnly))
        fnJsEvaluate(jsEngine, jsFile.readAll());
}

} // namespace

CommandExecScript::CommandExecScript(IAppContext* context, QJSEngine* jsEngine)
    : Command(context),
      m_jsEngine(jsEngine)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Execute Script..."));
    //action->setToolTip(Command::tr("Options"));
    this->setAction(action);
}

void CommandExecScript::execute()
{
    auto strFilePath = QFileDialog::getOpenFileName(
        this->widgetMain(),
        Command::tr("Choose JavaScript file"),
        QString(/*dir*/),
        Command::tr("Script files(*.js)")
    );
    if (strFilePath.isEmpty())
        return;

    evaluateScript(m_jsEngine, filepathFrom(strFilePath));
    AppModule::get()->prependRecentScript(filepathFrom(strFilePath));
}

CommandExecRecentScript::CommandExecRecentScript(IAppContext* context, QJSEngine* jsEngine)
    : Command(context),
      m_jsEngine(jsEngine)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Execute Recent Script"));
    this->setAction(action);
}

CommandExecRecentScript::CommandExecRecentScript(
        IAppContext* context, QMenu* containerMenu, QJSEngine* jsEngine
    )
    : CommandExecRecentScript(context, jsEngine)
{
    QObject::connect(containerMenu, &QMenu::aboutToShow, this, &CommandExecRecentScript::recreateEntries);
}

void CommandExecRecentScript::execute()
{
}

void CommandExecRecentScript::recreateEntries()
{
    QMenu* menu = this->action()->menu();
    if (!menu)
        menu = new QMenu(this->widgetMain());

    menu->clear();
    menu->setToolTipsVisible(true);
    int idFile = 0;
    auto appModule = AppModule::get();
    const RecentScripts& recentScripts = appModule->properties()->recentScripts;
    for (const RecentScript& recentScript : recentScripts) {
        const QString strFileName = filepathTo<QString>(recentScript.filepath.filename());
        const QString strEntryRecentScript = Command::tr("%1 | %2").arg(++idFile).arg(strFileName);
        auto action = menu->addAction(strEntryRecentScript, this, [=]{
            evaluateScript(m_jsEngine, recentScript.filepath);
            AppModule::get()->prependRecentScript(recentScript.filepath);
        });
        QDateTime dateTimeLastExec;
        dateTimeLastExec.setTimeSpec(Qt::UTC);
        dateTimeLastExec.setSecsSinceEpoch(recentScript.lastExecutionDateTime);
        dateTimeLastExec = dateTimeLastExec.toTimeSpec(Qt::LocalTime);
        action->setToolTip(
            Command::tr("%1\n%2 lines\nLast executed: %3\n%4")
                .arg(QDir::toNativeSeparators(filepathTo<QString>(recentScript.filepath)))
                .arg(RecentScript::lineCount(recentScript.filepath))
                .arg(QStringUtils::dateTimeText(dateTimeLastExec, AppModule::get()->qtLocale()))
                .arg(Command::tr("Executed %n time(s)", nullptr, recentScript.executionCount))
        );
    }

    if (!recentScripts.empty()) {
        menu->addSeparator();
        menu->addAction(Command::tr("Clear menu"), this, [=]{
            menu->clear();
            appModule->properties()->recentScripts.setValue({});
        });
    }

    this->action()->setMenu(menu);
}

} // namespace Mayo {

