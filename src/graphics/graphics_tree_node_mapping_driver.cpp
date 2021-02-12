/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_tree_node_mapping_driver.h"

#include "../base/document.h"

namespace Mayo {

std::unique_ptr<GraphicsTreeNodeMapping>
GraphicsShapeTreeNodeMappingDriver::createMapping(const DocumentTreeNode& entityTreeNode) const
{
    if (!entityTreeNode.isEntity())
        return {};

    if (!entityTreeNode.isValid())
        return {};

    if (!XCaf::isShape(entityTreeNode.label()))
        return {};

    int solidCount = 0;
    int faceCount = 0;
    const DocumentPtr doc = entityTreeNode.document();
    traverseTree(entityTreeNode.id(), doc->modelTree(), [&](TreeNodeId treeNodeId) {
        const TopoDS_Shape shape = XCaf::shape(doc->modelTree().nodeData(treeNodeId));
        if (shape.ShapeType() == TopAbs_SOLID)
            ++solidCount;
        else if (shape.ShapeType() == TopAbs_FACE)
            ++faceCount;
    });

    const TopAbs_ShapeEnum shapeType = solidCount > faceCount ? TopAbs_SOLID : TopAbs_FACE;
    return std::make_unique<GraphicsShapeTreeNodeMapping>(shapeType);
}

} // namespace Mayo
