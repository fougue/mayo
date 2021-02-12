/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree_builder.h"

#include "../base/document.h"
#include "../base/caf_utils.h"
#include "widget_model_tree.h"
#include "theme.h"

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

namespace Mayo {

WidgetModelTreeBuilder::~WidgetModelTreeBuilder()
{
}

void WidgetModelTreeBuilder::refreshTextTreeItem(const DocumentPtr& doc, QTreeWidgetItem* treeItem)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(doc->name()));
}

void WidgetModelTreeBuilder::refreshTextTreeItem(const DocumentTreeNode& node, QTreeWidgetItem* treeItem)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(node.label()));
}

QTreeWidgetItem* WidgetModelTreeBuilder::createTreeItem(const DocumentPtr& doc)
{
    auto treeItem = new QTreeWidgetItem;
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(doc->name()));
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::File));
    treeItem->setToolTip(0, doc->filePath());
    return treeItem;
}

QTreeWidgetItem* WidgetModelTreeBuilder::createTreeItem(const DocumentTreeNode& node)
{
    auto treeItem = new QTreeWidgetItem;
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(node.label()));
    treeItem->setFlags(treeItem->flags() | Qt::ItemIsUserCheckable);
    treeItem->setCheckState(0, Qt::Checked);
    return treeItem;
}

std::unique_ptr<WidgetModelTreeBuilder> WidgetModelTreeBuilder::clone() const
{
    return std::make_unique<WidgetModelTreeBuilder>();
}

QString WidgetModelTreeBuilder::labelText(const PropertyQString& propLabel)
{
    return WidgetModelTreeBuilder::labelText(propLabel.value());
}

QString WidgetModelTreeBuilder::labelText(const QString& label)
{
    return !label.isEmpty() ? label : WidgetModelTree::tr("<unnamed>");
}

QString WidgetModelTreeBuilder::labelText(const TDF_Label& label)
{
    return WidgetModelTreeBuilder::labelText(CafUtils::labelAttrStdName(label));
}

} // namespace Mayo
