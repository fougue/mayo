/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#pragma once

#include "../base/libtree.h"

class QPoint;
class QWidget;

namespace Mayo {

class GuiDocument;

// Pops up hide / show-only actions for the object under the cursor.
// If that object is not part of the current selection, it becomes the selection.
void popupObjectVisibilityMenu(
        QWidget* parent, GuiDocument* guiDoc, TreeNodeId clickedNodeId, const QPoint& globalPos
    );

} // namespace Mayo
