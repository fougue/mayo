/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_owner_ptr.h"
#include <TopAbs_ShapeEnum.hxx>
#include <memory>
#include <unordered_map>

namespace Mayo {

class DocumentTreeNode;

class GraphicsTreeNodeMapping {
public:
    virtual int selectionMode() const = 0;
    virtual std::vector<GraphicsOwnerPtr> findGraphicsOwners(const DocumentTreeNode& treeNode) const = 0;
    virtual bool mapGraphicsOwner(const GraphicsOwnerPtr& gfxOwner) = 0;
};

class GraphicsShapeTreeNodeMapping : public GraphicsTreeNodeMapping {
public:
    GraphicsShapeTreeNodeMapping(TopAbs_ShapeEnum shapeType);

    int selectionMode() const override;
    std::vector<GraphicsOwnerPtr> findGraphicsOwners(const DocumentTreeNode& treeNode) const override;
    bool mapGraphicsOwner(const GraphicsOwnerPtr& gfxOwner) override;

private:
    std::unordered_map<int, GraphicsOwnerPtr> m_mapGfxOwner;
    TopAbs_ShapeEnum m_shapeType;
};

} // namespace Mayo
