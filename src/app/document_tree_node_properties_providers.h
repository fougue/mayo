/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property.h"
#include <memory>
#include <vector>

namespace Mayo {

class DocumentTreeNode;
class PropertyGroup;

// Interface
// Provides relevant properties for the data associated to a model tree node
class DocumentTreeNodePropertiesProvider {
public:
    virtual ~DocumentTreeNodePropertiesProvider() = default;
    virtual bool supports(const DocumentTreeNode& treeNode) const = 0;
    virtual std::unique_ptr<PropertyGroup> properties(const DocumentTreeNode& treeNode) const = 0;
    virtual TextId subGroupLabelFromId(uint64_t id) const = 0;
};

// Provides relevant properties for tree node pointing to XCAF data
class XCaf_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroup> properties(const DocumentTreeNode& treeNode) const override;
    TextId subGroupLabelFromId(uint64_t id) const override;

private:
    class Properties;
};

// Provides relevant properties for tree node pointing to mesh data
class Mesh_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroup> properties(const DocumentTreeNode& treeNode) const override;
    TextId subGroupLabelFromId(uint64_t id) const override;

private:
    class Properties;
};

// Provides relevant properties for tree node pointing to point cloud data
class PointCloud_DocumentTreeNodePropertiesProvider : public DocumentTreeNodePropertiesProvider {
public:
    bool supports(const DocumentTreeNode& treeNode) const override;
    std::unique_ptr<PropertyGroup> properties(const DocumentTreeNode& treeNode) const override;
    TextId subGroupLabelFromId(uint64_t id) const override;

private:
    class Properties;
};

} // namespace Mayo
