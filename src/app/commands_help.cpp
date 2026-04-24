/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "commands_help.h"

#include "app_module.h"
#include "dialog_about.h"
#include "library_info.h"
#include "qtwidgets_utils.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtCore/QCoreApplication>

namespace Mayo {

CommandReportBug::CommandReportBug(IAppContext* context)
    : Command(context)
{
    auto action = this->createAction();
    action->setText(Command::tr("Report Bug"));
}

void CommandReportBug::execute()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/fougue/mayo/issues")));
}

CommandAbout::CommandAbout(IAppContext* context)
    : Command(context)
{
    auto action = this->createAction();
    action->setMenuRole(QAction::AboutRole);
    action->setText(Command::tr("About %1").arg(QCoreApplication::applicationName()));
}

void CommandAbout::execute()
{
    auto dlg = new DialogAbout(this->widgetMain());
    for (const LibraryInfo& libInfo : AppModule::get()->libraryInfoArray())
        dlg->addLibraryInfo(libInfo.name, libInfo.version);

    QtWidgetsUtils::asyncDialogExec(dlg);
}

} // namespace Mayo
