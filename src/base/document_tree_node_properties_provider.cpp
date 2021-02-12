/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_tree_node_properties_provider.h"
#include "document_tree_node.h"

namespace Mayo {

void DocumentTreeNodePropertiesProviderTable::addProvider(ProviderPtr provider)
{
    m_vecProvider.push_back(std::move(provider));
}

std::unique_ptr<PropertyGroupSignals>
DocumentTreeNodePropertiesProviderTable::properties(const DocumentTreeNode& treeNode) const
{
    for (const ProviderPtr& provider : m_vecProvider) {
        if (provider->supports(treeNode))
            return provider->properties(treeNode);
    }

    return std::unique_ptr<PropertyGroupSignals>();
}

} // namespace Mayo
