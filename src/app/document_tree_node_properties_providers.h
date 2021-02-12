/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_tree_node_properties_provider.h"
#include "../base/property_builtins.h"

#include <TDF_Label.hxx>

namespace Mayo {

class XCaf_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const override;

private:
    class Properties;
};

class Mesh_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroupSignals> properties(const DocumentTreeNode& treeNode) const override;

private:
    class Properties;
};

} // namespace Mayo
