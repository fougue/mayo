/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree_builder_mesh.h"
#include "../graphics/graphics_mesh_object_driver.h"
#include "theme.h"
#include "widget_model_tree.h"

#include <QtWidgets/QTreeWidgetItem>

namespace Mayo {

bool WidgetModelTreeBuilder_Mesh::supportsDocumentTreeNode(const DocumentTreeNode& node) const
{
    return GraphicsMeshObjectDriver::meshSupportStatus(node.label()) == GraphicsObjectDriver::Support::Complete;
}

QTreeWidgetItem* WidgetModelTreeBuilder_Mesh::createTreeItem(const DocumentTreeNode& node)
{
    Expects(this->supportsDocumentTreeNode(node));

    auto treeItem = WidgetModelTreeBuilder::createTreeItem(node);
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::ItemMesh));
    return treeItem;
}

std::unique_ptr<WidgetModelTreeBuilder> WidgetModelTreeBuilder_Mesh::clone() const
{
    return std::make_unique<WidgetModelTreeBuilder_Mesh>();
}

} // namespace Mayo
