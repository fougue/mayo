/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property.h"
#include "property_builtins.h"
#include <memory>

namespace Mayo {

class DocumentTreeNode;
class PropertyOwnerSignals;

class DocumentTreeNodePropertiesProvider {
public:
    virtual bool supports(const DocumentTreeNode& treeNode) const = 0;
    virtual std::unique_ptr<PropertyOwnerSignals> properties(const DocumentTreeNode& treeNode) const = 0;
};

} // namespace Mayo
