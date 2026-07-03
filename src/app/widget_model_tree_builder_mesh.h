/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
