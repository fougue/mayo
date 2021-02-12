/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include <memory>

namespace Mayo {

class DocumentTreeNode;
class PropertyGroupSignals;

class DocumentTreeNodePropertiesProvider {
public:
    virtual bool supports(const DocumentTreeNode& treeNode) const = 0;
    virtual std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const = 0;
};

class DocumentTreeNodePropertiesProviderTable {
public:
    using ProviderPtr = std::unique_ptr<DocumentTreeNodePropertiesProvider>;

    void addProvider(ProviderPtr provider);
    Span<const ProviderPtr> providers() const { return m_vecProvider; }

    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const;

private:
    std::vector<ProviderPtr> m_vecProvider;
};

} // namespace Mayo
