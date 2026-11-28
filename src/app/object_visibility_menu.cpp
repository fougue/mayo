/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "object_visibility_menu.h"

#include "../base/application_item_selection_model.h"
#include "../gui/gui_application.h"
#include "../gui/gui_document.h"
#include "commands_display.h"
#include "qtwidgets_utils.h"
#include "theme.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

#include <vector>

namespace Mayo {

namespace {

QString trCommand(const char* text)
{
    return QApplication::translate("Mayo::Command", text);
}

void selectClickedNode(GuiDocument* guiDoc, TreeNodeId nodeId)
{
    const ApplicationItem clickedItem({ guiDoc->document(), nodeId });
    ApplicationItemSelectionModel* selection = guiDoc->guiApplication()->selectionModel();
    if (selection->isSelected(clickedItem))
        return;

    selection->clear();
    selection->add(clickedItem);
}

bool selectionCanBeHidden(const GuiDocument* guiDoc)
{
    for (TreeNodeId nodeId : guiDoc->selectedNodeIds()) {
        if (guiDoc->nodeVisibleState(nodeId) != CheckState::Off)
            return true;
    }

    return false;
}

} // namespace

void popupObjectVisibilityMenu(
        QWidget* parent, GuiDocument* guiDoc, TreeNodeId clickedNodeId, const QPoint& globalPos
    )
{
    if (!guiDoc || clickedNodeId == 0)
        return;

    selectClickedNode(guiDoc, clickedNodeId);

    auto menu = new QMenu(parent);
    const bool clickedNodeIsHidden = guiDoc->nodeVisibleState(clickedNodeId) == CheckState::Off;
    const std::vector<TreeNodeId> clickedNodeIds{ clickedNodeId };

    auto actionHideSelection = menu->addAction(
        mayoTheme()->icon(Theme::Icon::VisibilityHideSelection), trCommand("Hide Selection")
    );
    actionHideSelection->setShortcut(hideSelectionShortcut());
    actionHideSelection->setShortcutContext(Qt::WidgetShortcut);
    actionHideSelection->setEnabled(selectionCanBeHidden(guiDoc));
    QObject::connect(actionHideSelection, &QAction::triggered, menu, [=]{
        guiDoc->hideNodes(guiDoc->selectedNodeIds());
    });

    auto actionHideThis = menu->addAction(
        mayoTheme()->icon(Theme::Icon::VisibilityHideSelection), trCommand("Hide Only This Item")
    );
    actionHideThis->setEnabled(!clickedNodeIsHidden);
    QObject::connect(actionHideThis, &QAction::triggered, menu, [=]{
        guiDoc->hideNodes(clickedNodeIds);
    });

    menu->addSeparator();

    auto actionShowOnlySelection = menu->addAction(
        mayoTheme()->icon(Theme::Icon::VisibilityShowSelectionOnly), trCommand("Show Only Selection")
    );
    actionShowOnlySelection->setShortcut(showOnlySelectionShortcut());
    actionShowOnlySelection->setShortcutContext(Qt::WidgetShortcut);
    actionShowOnlySelection->setEnabled(!guiDoc->selectedNodeIds().empty());
    QObject::connect(actionShowOnlySelection, &QAction::triggered, menu, [=]{
        guiDoc->showOnlyNodes(guiDoc->selectedNodeIds());
    });

    auto actionShowOnlyThis = menu->addAction(
        mayoTheme()->icon(Theme::Icon::VisibilityShowSelectionOnly), trCommand("Show Only This Item")
    );
    QObject::connect(actionShowOnlyThis, &QAction::triggered, menu, [=]{
        guiDoc->showOnlyNodes(clickedNodeIds);
    });

    QtWidgetsUtils::asyncMenuExec(menu, globalPos);
}

} // namespace Mayo
