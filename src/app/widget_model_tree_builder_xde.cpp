#include "widget_model_tree_builder_xde.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/settings.h"
#include "../base/xcaf.h"
#include "theme.h"
#include "widget_model_tree.h"

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <unordered_map>
#include <unordered_set>

namespace Mayo {

namespace Internal {

static QIcon shapeIcon(const TDF_Label& label)
{
    if (XCaf::isShapeAssembly(label))
        return mayoTheme()->icon(Theme::Icon::XdeAssembly);
    else if (XCaf::isShapeReference(label))
        return QIcon(":/images/xde_reference_16.png"); // TODO move in Theme
    else if (XCaf::isShapeSimple(label))
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

bool WidgetModelTreeBuilder_Xde::supportsDocumentTreeNode(const DocumentTreeNode& node) const
{
    return XCaf::isShape(node.label());
}

void WidgetModelTreeBuilder_Xde::refreshTextTreeItem(
        const DocumentTreeNode& node, QTreeWidgetItem* treeItem)
{
    const TDF_Label labelNode = node.label();
    TDF_LabelSequence seqLabelRefresh;
    if (XCaf::isShapeReference(labelNode) && m_refItemTextTemplate.contains("%referred")) {
        XCAFDoc_ShapeTool::GetUsers(
                    XCaf::shapeReferred(labelNode),
                    seqLabelRefresh,
                    false /* don't get sub children */);
    }
    else {
        seqLabelRefresh.Append(labelNode);
    }

    for (const TDF_Label& labelRefresh : seqLabelRefresh) {
        QTreeWidgetItem* treeItemLabel = this->findTreeItem(treeItem, labelRefresh);
        if (treeItemLabel)
            this->refreshXdeAssemblyNodeItemText(treeItemLabel);
    }
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::createTreeItem(const DocumentTreeNode& node)
{
    Expects(this->supportsDocumentTreeNode(node));
    return this->buildXdeTree(nullptr, node);
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::guiCreateXdeTreeNode(
        QTreeWidgetItem* guiParentNode, const DocumentTreeNode& node)
{
    auto guiNode = new QTreeWidgetItem(guiParentNode);
    const QString stdName = CafUtils::labelAttrStdName(node.label());
    guiNode->setText(0, stdName);
    WidgetModelTree::setDocumentTreeNode(guiNode, node);
    const QIcon icon = Internal::shapeIcon(node.label());
    if (!icon.isNull())
        guiNode->setIcon(0, icon);

    return guiNode;
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::buildXdeTree(
        QTreeWidgetItem* treeItem, const DocumentTreeNode& node)
{
    Expects(node.isEntity());

    std::unordered_map<TreeNodeId, QTreeWidgetItem*> mapNodeIdToTreeItem;
    std::unordered_set<TreeNodeId> setReferenceNodeId;
    const DocumentPtr doc = node.document();
    const Tree<TDF_Label>& modelTree = doc->modelTree();
    deepForeachTreeNode(node.id(), modelTree, [&](TreeNodeId itNodeId) {
        const TreeNodeId nodeParentId = modelTree.nodeParent(itNodeId);
        auto itParentFound = mapNodeIdToTreeItem.find(nodeParentId);
        QTreeWidgetItem* guiParentNode =
                itParentFound != mapNodeIdToTreeItem.end() ? itParentFound->second : treeItem;
        if (m_isMergeXdeReferredShapeOn) {
            const TDF_Label& nodeLabel = modelTree.nodeData(itNodeId);
            if (XCaf::isShapeReference(nodeLabel)) {
                mapNodeIdToTreeItem.insert({ itNodeId, guiParentNode });
                setReferenceNodeId.insert(itNodeId);
            }
            else {
                auto guiNode = new QTreeWidgetItem(guiParentNode);
                QString guiNodeText = CafUtils::labelAttrStdName(nodeLabel);
                TreeNodeId guiNodeId = itNodeId;
                if (setReferenceNodeId.find(nodeParentId) != setReferenceNodeId.cend()) {
                    const TDF_Label& refLabel = modelTree.nodeData(nodeParentId);
                    guiNodeText = this->referenceItemText(refLabel, nodeLabel);
                    guiNodeId = nodeParentId;
                    if (!guiParentNode)
                        mapNodeIdToTreeItem.insert_or_assign(nodeParentId, guiNode);
                }

                guiNode->setText(0, guiNodeText);
                WidgetModelTree::setDocumentTreeNode(guiNode, DocumentTreeNode(doc, guiNodeId));
                const QIcon icon = Internal::shapeIcon(nodeLabel);
                if (!icon.isNull())
                    guiNode->setIcon(0, icon);

                mapNodeIdToTreeItem.insert({ itNodeId, guiNode });
            }
        }
        else {
            auto guiNode = ThisType::guiCreateXdeTreeNode(guiParentNode, { doc, itNodeId });
            mapNodeIdToTreeItem.insert({ itNodeId, guiNode });
        }
    });

    return mapNodeIdToTreeItem.find(node.id())->second;
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
        if (WidgetModelTree::holdsDocumentTreeNode(*it))
            this->refreshXdeAssemblyNodeItemText(*it);
    }
}

void WidgetModelTreeBuilder_Xde::loadConfiguration(const Settings* settings, const QString& keyGroup)
{
    const QString refTextTemplate =
            settings->valueAs<QString>(keyGroup + Internal::keyRefItemTextTemplate);
    this->setReferenceItemTextTemplate(
                !refTextTemplate.isEmpty() ? refTextTemplate : Internal::textTemplateReferenceOnly);
}

void WidgetModelTreeBuilder_Xde::saveConfiguration(Settings* settings, const QString& keyGroup)
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

std::unique_ptr<WidgetModelTreeBuilder> WidgetModelTreeBuilder_Xde::clone() const
{
    auto builder = std::make_unique<WidgetModelTreeBuilder_Xde>();
    builder->m_isMergeXdeReferredShapeOn = this->m_isMergeXdeReferredShapeOn;
    builder->m_refItemTextTemplate = this->m_refItemTextTemplate;
    return builder;
}

void WidgetModelTreeBuilder_Xde::refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item)
{
    const DocumentTreeNode docTreeNode = WidgetModelTree::documentTreeNode(item);
    const TDF_Label label = docTreeNode.label();
    if (XCaf::isShapeReference(label)) {
        const TDF_Label& refLabel = label;
        const TDF_Label referredLabel = XCaf::shapeReferred(refLabel);
        const QString itemText = this->referenceItemText(refLabel, referredLabel);
        item->setText(0, itemText);
    }
    else {
        item->setText(0, CafUtils::labelAttrStdName(label));
    }
}

QString WidgetModelTreeBuilder_Xde::referenceItemText(
        const TDF_Label& refLabel, const TDF_Label& referredLabel) const
{
    const QString refName = CafUtils::labelAttrStdName(refLabel).trimmed();
    const QString referredName = CafUtils::labelAttrStdName(referredLabel).trimmed();
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
        const DocumentTreeNode node = WidgetModelTree::documentTreeNode(*it);
        if (node.label() == label)
            return *it;
    }

    return nullptr;
}

} // namespace Mayo
