/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_tree_node_mapping.h"

namespace Mayo {

class GraphicsTreeNodeMappingDriver {
public:
    using MappingPtr = std::unique_ptr<GraphicsTreeNodeMapping>;
    virtual MappingPtr createMapping(const DocumentTreeNode& entityTreeNode) const = 0;
};

class GraphicsShapeTreeNodeMappingDriver : public GraphicsTreeNodeMappingDriver {
public:
    MappingPtr createMapping(const DocumentTreeNode& entityTreeNode) const override;
};

} // namespace Mayo
