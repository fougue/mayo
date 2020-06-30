/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_owner_ptr.h"
#include <memory>
#include <unordered_map>
class TDF_Label;

namespace Mayo {

class DocumentTreeNode;

class GraphicsTreeNodeMapping {
public:
    virtual int selectionMode() const = 0;
    virtual std::vector<GraphicsOwnerPtr> findGraphicsOwners(const DocumentTreeNode& treeNode) const = 0;
    virtual bool mapGraphicsOwner(const GraphicsOwnerPtr& gfxOwner) = 0;
};

class GraphicsTreeNodeMappingDriver {
public:
    virtual bool supportsEntity(const TDF_Label& label) const = 0;
    virtual std::unique_ptr<GraphicsTreeNodeMapping> createMapping() const = 0;
};

class GraphicsShapeTreeNodeMapping : public GraphicsTreeNodeMapping {
public:
    int selectionMode() const override;
    std::vector<GraphicsOwnerPtr> findGraphicsOwners(const DocumentTreeNode& treeNode) const override;
    bool mapGraphicsOwner(const GraphicsOwnerPtr& gfxOwner) override;

private:
    std::unordered_map<int, GraphicsOwnerPtr> m_mapGfxOwner;
};

class GraphicsShapeTreeNodeMappingDriver : public GraphicsTreeNodeMappingDriver {
public:
    bool supportsEntity(const TDF_Label& label) const override;
    std::unique_ptr<GraphicsTreeNodeMapping> createMapping() const override;
};

} // namespace Mayo {
