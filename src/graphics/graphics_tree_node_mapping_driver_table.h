/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "graphics_tree_node_mapping_driver.h"
#include "../base/span.h"
#include <memory>
#include <vector>

namespace Mayo {

class GraphicsTreeNodeMappingDriverTable {
public:
    using DriverPtr = std::unique_ptr<GraphicsTreeNodeMappingDriver>;

    void addDriver(DriverPtr driver);
    Span<const DriverPtr> drivers() const { return m_vecDriver; }

    std::unique_ptr<GraphicsTreeNodeMapping> createMapping(const DocumentTreeNode& entityTreeNode) const;

private:
    std::vector<DriverPtr> m_vecDriver;
};

} // namespace Mayo
