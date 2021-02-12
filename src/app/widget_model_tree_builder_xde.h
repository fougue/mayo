/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "widget_model_tree_builder.h"
class TDF_Label;

namespace Mayo {

class WidgetModelTreeBuilder_Xde : public WidgetModelTreeBuilder {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::WidgetModelTreeBuilder_Xde)
public:
    bool supportsDocumentTreeNode(const DocumentTreeNode& node) const override;
    void refreshTextTreeItem(const DocumentTreeNode& node, QTreeWidgetItem* treeItem) override;
    QTreeWidgetItem* createTreeItem(const DocumentTreeNode& node) override;

    void registerGuiApplication(GuiApplication* guiApp) override;
    WidgetModelTree_UserActions createUserActions(QObject* parent) override;

    std::unique_ptr<WidgetModelTreeBuilder> clone() const override;

private:
    class Module;

    using ThisType = WidgetModelTreeBuilder_Xde;

    static QTreeWidgetItem* guiCreateXdeTreeNode(
            QTreeWidgetItem* guiParentNode, const DocumentTreeNode& node);

    QTreeWidgetItem* buildXdeTree(QTreeWidgetItem* treeItem, const DocumentTreeNode& node);
    void refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item);
    QString referenceItemText(const TDF_Label& instanceLabel, const TDF_Label& productLabel) const;
    QTreeWidgetItem* findTreeItem(QTreeWidgetItem* parentTreeItem, const TDF_Label& label) const;

    QByteArray instanceNameFormat() const;
    void setInstanceNameFormat(const QByteArray& format);

    Module* m_module = nullptr;
    bool m_isMergeXdeReferredShapeOn = true;
};

} // namespace Mayo
