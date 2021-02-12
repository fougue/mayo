/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "widget_model_tree_builder.h"

namespace Mayo {

class WidgetModelTreeBuilder_Mesh : public WidgetModelTreeBuilder {
public:
    bool supportsDocumentTreeNode(const DocumentTreeNode& node) const override;
    QTreeWidgetItem* createTreeItem(const DocumentTreeNode& node) override;
    std::unique_ptr<WidgetModelTreeBuilder> clone() const override;
};

} // namespace Mayo
