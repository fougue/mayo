/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "widget_model_tree_builder.h"

namespace Mayo {

class WidgetModelTreeBuilder_Mesh : public WidgetModelTreeBuilder {
public:
    bool supports(const DocumentItem* docItem) const override;
    void fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem) override;

    WidgetModelTreeBuilder* clone() const override;
};

} // namespace Mayo
