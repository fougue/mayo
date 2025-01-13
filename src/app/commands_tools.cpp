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
#include "app_module.h"
#include "dialog_inspect_xde.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "qtwidgets_utils.h"
#include "theme.h"

#include <QtWidgets/QWidget>

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

} // namespace Mayo {

