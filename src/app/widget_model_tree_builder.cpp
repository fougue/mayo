/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree_builder.h"

#include "../base/document.h"
#include "../base/document_item.h"
#include "widget_model_tree.h"
#include "theme.h"

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

namespace Mayo {

WidgetModelTreeBuilder::~WidgetModelTreeBuilder()
{
}

void WidgetModelTreeBuilder::refreshTextTreeItem(
        const Document* doc, QTreeWidgetItem* treeItem)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(doc->propertyLabel));
}

void WidgetModelTreeBuilder::refreshTextTreeItem(
        const DocumentItem* docItem, QTreeWidgetItem* treeItem)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(docItem->propertyLabel));
}

void WidgetModelTreeBuilder::refreshTextTreeItem(
        const DocumentItemNode&, QTreeWidgetItem*)
{
}

void WidgetModelTreeBuilder::fillTreeItem(QTreeWidgetItem* treeItem, Document* doc)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(doc->propertyLabel));
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::File));
    treeItem->setToolTip(0, doc->filePath());
}

void WidgetModelTreeBuilder::fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem)
{
    treeItem->setText(0, WidgetModelTreeBuilder::labelText(docItem->propertyLabel));
}

void WidgetModelTreeBuilder::loadConfiguration(const Settings*, const QString&)
{
}

void WidgetModelTreeBuilder::saveConfiguration(Settings*, const QString&)
{
}

std::vector<QAction*> WidgetModelTreeBuilder::createConfigurationActions(QObject*)
{
    std::vector<QAction*> vecAction;
    return vecAction;
}

WidgetModelTreeBuilder* WidgetModelTreeBuilder::clone() const
{
    return new WidgetModelTreeBuilder;
}

QString WidgetModelTreeBuilder::labelText(const PropertyQString& propLabel)
{
    const QString label = propLabel.value();
    return !label.isEmpty() ? label : WidgetModelTree::tr("<unnamed>");
}

} // namespace Mayo
