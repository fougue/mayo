/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree_builder_mesh.h"

#include "../base/mesh_item.h"
#include "theme.h"
#include "widget_model_tree.h"

#include <QtWidgets/QTreeWidgetItem>

namespace Mayo {

bool WidgetModelTreeBuilder_Mesh::supports(const DocumentItem* docItem) const
{
    return sameType<MeshItem>(docItem);
}

void WidgetModelTreeBuilder_Mesh::fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem)
{
    WidgetModelTreeBuilder::fillTreeItem(treeItem, docItem);
    Q_ASSERT(this->supports(docItem));
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::ItemMesh));
}

WidgetModelTreeBuilder* WidgetModelTreeBuilder_Mesh::clone() const
{
    return new WidgetModelTreeBuilder_Mesh;
}

} // namespace Mayo
