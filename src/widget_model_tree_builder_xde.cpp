#include "widget_model_tree_builder_xde.h"

#include "theme.h"
#include "widget_model_tree.h"
#include "xde_document_item.h"

#include <QtCore/QSettings>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <unordered_map>
#include <unordered_set>

namespace Mayo {

namespace Internal {

static QIcon shapeIcon(const TDF_Label& label)
{
    if (XdeDocumentItem::isShapeAssembly(label))
        return mayoTheme()->icon(Theme::Icon::XdeAssembly);
    else if (XdeDocumentItem::isShapeReference(label))
        return QIcon(":/images/xde_reference_16.png"); // TODO move in Theme
    else if (XdeDocumentItem::isShapeSimple(label))
        return mayoTheme()->icon(Theme::Icon::XdeSimpleShape);

    return QIcon();
}

static const char textTemplateReferenceOnly[] = "%instance";
static const char textTemplateReferredOnly[] = "%referred";
// UTF8 rightwards arrow : \xe2\x86\x92
static const char textTemplateReferenceAndReferred[] = "%instance \xe2\x86\x92 %referred";

static const char keyRefItemTextTemplate[] = "/xde/reference_item_text_template";

} // namespace Internal

WidgetModelTreeBuilder_Xde::WidgetModelTreeBuilder_Xde()
    : m_refItemTextTemplate("%instance")
{
}

bool WidgetModelTreeBuilder_Xde::supports(const DocumentItem* docItem) const
{
    return sameType<XdeDocumentItem>(docItem);
}

void WidgetModelTreeBuilder_Xde::refreshTextTreeItem(
        const DocumentItemNode& node, QTreeWidgetItem* treeItemDocItem)
{
    const TDF_Label labelNode = XdeDocumentItem::label(node);
    TDF_LabelSequence seqLabelRefresh;
    if (XdeDocumentItem::isShapeReference(labelNode)
            && m_refItemTextTemplate.contains("%referred"))
    {
        XCAFDoc_ShapeTool::GetUsers(
                    XdeDocumentItem::shapeReferred(labelNode),
                    seqLabelRefresh,
                    false /* don't get sub children */);
    }
    else {
        seqLabelRefresh.Append(labelNode);
    }

    for (const TDF_Label& labelRefresh : seqLabelRefresh) {
        QTreeWidgetItem* treeItemLabel = this->findTreeItem(treeItemDocItem, labelRefresh);
        if (treeItemLabel)
            this->refreshXdeAssemblyNodeItemText(treeItemLabel);
    }
}

void WidgetModelTreeBuilder_Xde::fillTreeItem(QTreeWidgetItem* treeItem, DocumentItem* docItem)
{
    WidgetModelTreeBuilder::fillTreeItem(treeItem, docItem);
    Q_ASSERT(this->supports(docItem));
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::ItemXde));
    auto xdeDocItem = static_cast<XdeDocumentItem*>(docItem);
    this->buildXdeTree(treeItem, xdeDocItem);
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::guiCreateXdeTreeNode(
        QTreeWidgetItem* guiParentNode, TreeNodeId nodeId, XdeDocumentItem* docItem)
{
    auto guiNode = new QTreeWidgetItem(guiParentNode);
    const QString stdName = docItem->findLabelName(nodeId);
    guiNode->setText(0, stdName);
    WidgetModelTree::setDocumentItemNode(guiNode, DocumentItemNode(docItem, nodeId));
    const QIcon icon = Internal::shapeIcon(docItem->assemblyTree().nodeData(nodeId));
    if (!icon.isNull())
        guiNode->setIcon(0, icon);

    return guiNode;
}

void WidgetModelTreeBuilder_Xde::buildXdeTree(
        QTreeWidgetItem* treeItem, XdeDocumentItem* docItem)
{
    std::unordered_map<TreeNodeId, QTreeWidgetItem*> mapNodeIdToTreeItem;
    std::unordered_set<TreeNodeId> setRefNodeId;
    const Tree<TDF_Label>& asmTree = docItem->assemblyTree();
    deepForeachTreeNode(asmTree, [&](TreeNodeId nodeId) {
        const TreeNodeId nodeParentId = asmTree.nodeParent(nodeId);
        const TDF_Label& nodeLabel = asmTree.nodeData(nodeId);
        auto itParentFound = mapNodeIdToTreeItem.find(nodeParentId);
        QTreeWidgetItem* guiParentNode =
                itParentFound != mapNodeIdToTreeItem.end() ?
                    itParentFound->second : treeItem;
        if (m_isMergeXdeReferredShapeOn) {
            if (XdeDocumentItem::isShapeReference(nodeLabel)) {
                mapNodeIdToTreeItem.insert({ nodeId, guiParentNode });
                setRefNodeId.insert(nodeId);
            }
            else {
                auto guiNode = new QTreeWidgetItem(guiParentNode);
                QString guiNodeText = XdeDocumentItem::findLabelName(nodeLabel);
                XdeDocumentItem::AssemblyNodeId guiNodeId = nodeId;
                if (setRefNodeId.find(nodeParentId) != setRefNodeId.cend()) {
                    const TDF_Label& refLabel = asmTree.nodeData(nodeParentId);
                    guiNodeText = this->referenceItemText(refLabel, nodeLabel);
                    guiNodeId = nodeParentId;
                }

                guiNode->setText(0, guiNodeText);
                WidgetModelTree::setDocumentItemNode(
                            guiNode, DocumentItemNode(docItem, guiNodeId));
                const QIcon icon = Internal::shapeIcon(nodeLabel);
                if (!icon.isNull())
                    guiNode->setIcon(0, icon);

                mapNodeIdToTreeItem.insert({ nodeId, guiNode });
            }
        }
        else {
            auto guiNode = ThisType::guiCreateXdeTreeNode(guiParentNode, nodeId, docItem);
            mapNodeIdToTreeItem.insert({ nodeId, guiNode });
        }
    });
}

const QString& WidgetModelTreeBuilder_Xde::referenceItemTextTemplate() const
{
    return m_refItemTextTemplate;
}

void WidgetModelTreeBuilder_Xde::setReferenceItemTextTemplate(const QString& textTemplate)
{
    if (textTemplate == m_refItemTextTemplate)
        return;

    m_refItemTextTemplate = textTemplate;
    for (QTreeWidgetItemIterator it(this->treeWidget()); *it; ++it) {
        if (WidgetModelTree::holdsDocumentItemNode(*it))
            this->refreshXdeAssemblyNodeItemText(*it);
    }
}

void WidgetModelTreeBuilder_Xde::loadConfiguration(
        const QSettings* settings, const QString& keyGroup)
{
    const QString refTextTemplate =
            settings->value(
                keyGroup + Internal::keyRefItemTextTemplate,
                Internal::textTemplateReferenceOnly)
            .toString();
    this->setReferenceItemTextTemplate(refTextTemplate);
}

void WidgetModelTreeBuilder_Xde::saveConfiguration(
        QSettings* settings, const QString& keyGroup)
{
    settings->setValue(
                keyGroup + Internal::keyRefItemTextTemplate,
                m_refItemTextTemplate);
}

std::vector<QAction*> WidgetModelTreeBuilder_Xde::createConfigurationActions(QObject* parent)
{
    // Note
    // UTF8 rightwards arrow : \xe2\x86\x92

    std::vector<QAction*> vecAction;
    vecAction.push_back(new QAction(tr("Show only name of reference entities"), parent));
    vecAction.push_back(new QAction(tr("Show only name of referred entities"), parent));
    vecAction.push_back(new QAction(tr("Show name of both reference and referred entities"), parent));
    vecAction.at(0)->setData(Internal::textTemplateReferenceOnly);
    vecAction.at(1)->setData(Internal::textTemplateReferredOnly);
    vecAction.at(2)->setData(Internal::textTemplateReferenceAndReferred);
    auto group = new QActionGroup(parent);
    group->setExclusive(true);
    for (QAction* action : vecAction) {
        action->setCheckable(true);
        group->addAction(action);
        if (action->data().toString() == m_refItemTextTemplate)
            action->setChecked(true);
    }

    QObject::connect(group, &QActionGroup::triggered, [=](QAction* action) {
        this->setReferenceItemTextTemplate(action->data().toString());
    });

    return vecAction;
}

WidgetModelTreeBuilder* WidgetModelTreeBuilder_Xde::clone() const
{
    auto builder = new WidgetModelTreeBuilder_Xde;
    builder->m_isMergeXdeReferredShapeOn = this->m_isMergeXdeReferredShapeOn;
    builder->m_refItemTextTemplate = this->m_refItemTextTemplate;
    return builder;
}

void WidgetModelTreeBuilder_Xde::refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item)
{
    const DocumentItemNode docItemNode = WidgetModelTree::documentItemNode(item);
    if (!sameType<XdeDocumentItem>(docItemNode.documentItem))
        return;

    const TDF_Label label = XdeDocumentItem::label(docItemNode);
    if (XdeDocumentItem::isShapeReference(label)) {
        const TDF_Label& refLabel = label;
        const TDF_Label referredLabel = XdeDocumentItem::shapeReferred(refLabel);
        const QString itemText = this->referenceItemText(refLabel, referredLabel);
        item->setText(0, itemText);
    }
    else {
        item->setText(0, XdeDocumentItem::findLabelName(label));
    }
}

QString WidgetModelTreeBuilder_Xde::referenceItemText(
        const TDF_Label& refLabel, const TDF_Label& referredLabel) const
{
    const QString refName = XdeDocumentItem::findLabelName(refLabel).trimmed();
    const QString referredName = XdeDocumentItem::findLabelName(referredLabel).trimmed();
    QString itemText = m_refItemTextTemplate;
    itemText.replace("%instance", refName)
            .replace("%referred", referredName);
    return itemText;
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::findTreeItem(
        QTreeWidgetItem* parentTreeItem, const TDF_Label& label) const
{
    if (!parentTreeItem)
        return nullptr;

    for (QTreeWidgetItemIterator it(parentTreeItem); *it; ++it) {
        const DocumentItemNode docItemNode = WidgetModelTree::documentItemNode(*it);
        if (XdeDocumentItem::label(docItemNode) == label)
            return *it;
    }

    return nullptr;
}

} // namespace Mayo
