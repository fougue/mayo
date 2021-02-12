/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "graphics_tree_node_mapping_driver_table.h"

namespace Mayo {

void GraphicsTreeNodeMappingDriverTable::addDriver(DriverPtr driver)
{
    m_vecDriver.push_back(std::move(driver));
}

std::unique_ptr<GraphicsTreeNodeMapping>
GraphicsTreeNodeMappingDriverTable::createMapping(const DocumentTreeNode& entityTreeNode) const
{
    for (const DriverPtr& driver : m_vecDriver) {
        std::unique_ptr<GraphicsTreeNodeMapping> ptr = driver->createMapping(entityTreeNode);
        if (ptr)
            return ptr;
    }

    return {};
}

} // namespace Mayo
