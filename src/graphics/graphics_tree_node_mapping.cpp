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

int GraphicsShapeTreeNodeMapping::selectionMode() const
{
    return AIS_Shape::SelectionMode(TopAbs_SOLID);
}

std::vector<GraphicsOwnerPtr>
GraphicsShapeTreeNodeMapping::findGraphicsOwners(const DocumentTreeNode& treeNode) const
{
    const TopLoc_Location shapeLoc = treeNode.document()->xcaf().shapeAbsoluteLocation(treeNode.id());
    const TopoDS_Shape shape = XCaf::shape(treeNode.label()).Located(shapeLoc);
    std::vector<TopoDS_Shape> vecSolid;
    if (BRepUtils::moreComplex(shape.ShapeType(), TopAbs_SOLID)) {
        BRepUtils::forEachSubShape(shape, TopAbs_SOLID, [&](const TopoDS_Shape& solid) {
            vecSolid.push_back(solid);
        });
    }
    else if (shape.ShapeType() == TopAbs_SOLID) {
        vecSolid.push_back(TopoDS::Solid(shape));
    }

    std::vector<GraphicsOwnerPtr> vecGfxOwner;
    for (const TopoDS_Shape& shape : vecSolid) {
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

    auto result = m_mapGfxOwner.emplace(BRepUtils::hashCode(brepOwner->Shape()), brepOwner);
    return result.second;
}

bool GraphicsShapeTreeNodeMappingDriver::supportsEntity(const TDF_Label& label) const
{
    return XCaf::isShape(label);
}

std::unique_ptr<GraphicsTreeNodeMapping> GraphicsShapeTreeNodeMappingDriver::createMapping() const
{
    return std::make_unique<GraphicsShapeTreeNodeMapping>();
}

} // namespace Mayo
