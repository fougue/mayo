/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/libtree.h"
#include "widget_model_tree_builder.h"
#include <QtCore/QCoreApplication>
class TDF_Label;

namespace Mayo {

class XdeDocumentItem;

class WidgetModelTreeBuilder_Xde : public WidgetModelTreeBuilder {
    Q_DECLARE_TR_FUNCTIONS(WidgetModelTreeBuilder_Xde)
public:
    WidgetModelTreeBuilder_Xde();

    bool supports(const DocumentItem* docItem) const override;
    void refreshTextTreeItem(const DocumentItemNode& node, QTreeWidgetItem* treeItemDocItem) override;
    void fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem) override;

    void loadConfiguration(const Settings* settings, const QString& keyGroup) override;
    void saveConfiguration(Settings* settings, const QString& keyGroup) override;
    std::vector<QAction*> createConfigurationActions(QObject* parent) override;

    WidgetModelTreeBuilder* clone() const override;

    const QString& referenceItemTextTemplate() const;
    void setReferenceItemTextTemplate(const QString& textTemplate);

private:
    using ThisType = WidgetModelTreeBuilder_Xde;
    using ParentType = WidgetModelTreeBuilder;

    static QTreeWidgetItem* guiCreateXdeTreeNode(
            QTreeWidgetItem* guiParentNode,
            TreeNodeId nodeId,
            XdeDocumentItem* docItem);

    void buildXdeTree(QTreeWidgetItem* treeItem, XdeDocumentItem* docItem);
    void refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item);
    QString referenceItemText(
            const TDF_Label& refLabel,
            const TDF_Label& referredLabel) const;
    QTreeWidgetItem* findTreeItem(
            QTreeWidgetItem* parentTreeItem, const TDF_Label& label) const;

    bool m_isMergeXdeReferredShapeOn = true;
    QString m_refItemTextTemplate;
};

} // namespace Mayo
