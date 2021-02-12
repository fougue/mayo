/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/document_ptr.h"
#include "../base/document_tree_node.h"
#include "../base/property_builtins.h"
#include "widget_model_tree.h"
#include <vector>
#include <QtCore/QString>
class QAction;
class QObject;
class QTreeWidget;
class QTreeWidgetItem;

namespace Mayo {

// TODO Rename Builder -> Extension ?
class WidgetModelTreeBuilder {
public:
    virtual ~WidgetModelTreeBuilder();

    virtual bool supportsDocument(const DocumentPtr&) const { return true; }
    virtual bool supportsDocumentTreeNode(const DocumentTreeNode&) const { return true; }

    virtual void refreshTextTreeItem(const DocumentPtr& doc, QTreeWidgetItem* treeItem);
    virtual void refreshTextTreeItem(const DocumentTreeNode& node, QTreeWidgetItem* treeItem);

    virtual QTreeWidgetItem* createTreeItem(const DocumentPtr& doc);
    virtual QTreeWidgetItem* createTreeItem(const DocumentTreeNode& node);

    QTreeWidget* treeWidget() const { return m_treeWidget; }
    void setTreeWidget(QTreeWidget* tree) { m_treeWidget = tree; }

    virtual void registerGuiApplication(GuiApplication* /*guiApp*/) {}

    virtual WidgetModelTree_UserActions createUserActions(QObject* /*parent*/) { return {}; }

    virtual std::unique_ptr<WidgetModelTreeBuilder> clone() const;

protected:
    static QString labelText(const PropertyQString& propLabel);
    static QString labelText(const QString& label);
    static QString labelText(const TDF_Label& label);

private:
    QTreeWidget* m_treeWidget = nullptr;
};

} // namespace Mayo
