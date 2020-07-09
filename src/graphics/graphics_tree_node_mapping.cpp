/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_tree_node_mapping.h"

#include "../base/brep_utils.h"
#include "../base/document.h"
#include "../base/document_tree_node.h"

#include <AIS_Shape.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopoDS_Solid.hxx>

namespace Mayo {

GraphicsShapeTreeNodeMapping::GraphicsShapeTreeNodeMapping(TopAbs_ShapeEnum shapeType)
    : m_shapeType(shapeType)
{
}

int GraphicsShapeTreeNodeMapping::selectionMode() const
{
    return AIS_Shape::SelectionMode(m_shapeType);
}

std::vector<GraphicsOwnerPtr>
GraphicsShapeTreeNodeMapping::findGraphicsOwners(const DocumentTreeNode& treeNode) const
{
    const TopLoc_Location shapeLoc = treeNode.document()->xcaf().shapeAbsoluteLocation(treeNode.id());
    const TopoDS_Shape shape = XCaf::shape(treeNode.label()).Located(shapeLoc);
    std::vector<TopoDS_Shape> vecShape;
    if (BRepUtils::moreComplex(shape.ShapeType(), m_shapeType)) {
        BRepUtils::forEachSubShape(shape, m_shapeType, [&](const TopoDS_Shape& subShape) {
            vecShape.push_back(subShape);
        });
    }
    else if (shape.ShapeType() == m_shapeType) {
        vecShape.push_back(shape);
    }

    std::vector<GraphicsOwnerPtr> vecGfxOwner;
    for (const TopoDS_Shape& shape : vecShape) {
        auto it = m_mapGfxOwner.find(BRepUtils::hashCode(shape));
        if (it != m_mapGfxOwner.cend())
            vecGfxOwner.push_back(it->second);
    }

    return vecGfxOwner;
}

bool GraphicsShapeTreeNodeMapping::mapGraphicsOwner(const GraphicsOwnerPtr& gfxOwner)
{
    auto brepOwner = Handle_StdSelect_BRepOwner::DownCast(gfxOwner);
    if (brepOwner.IsNull())
        return false;

    if (brepOwner->Shape().ShapeType() != m_shapeType)
        return false;

    auto result = m_mapGfxOwner.emplace(BRepUtils::hashCode(brepOwner->Shape()), brepOwner);
    return result.second;
}

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
    deepForeachTreeNode(entityTreeNode.id(), doc->modelTree(), [&](TreeNodeId treeNodeId) {
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
