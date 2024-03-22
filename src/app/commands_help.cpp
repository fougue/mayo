/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "commands_help.h"

#include "dialog_about.h"
#include "library_info.h"
#include "qtwidgets_utils.h"
#include "../qtcommon/qstring_conv.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtCore/QCoreApplication>

namespace Mayo {

CommandReportBug::CommandReportBug(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("Report Bug"));
    this->setAction(action);
}

void CommandReportBug::execute()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/fougue/mayo/issues")));
}

CommandAbout::CommandAbout(IAppContext* context)
    : Command(context)
{
    auto action = new QAction(this);
    action->setText(Command::tr("About %1").arg(QCoreApplication::applicationName()));
    this->setAction(action);
}

void CommandAbout::execute()
{
    auto dlg = new DialogAbout(this->widgetMain());
    for (const auto& libInfo : LibraryInfoArray::get())
        dlg->addLibraryInfo(libInfo.name, libInfo.version);

    QtWidgetsUtils::asyncDialogExec(dlg);
}

} // namespace Mayo
