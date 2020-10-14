#include "widget_model_tree_builder_xde.h"

#include "../base/application.h"
#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../base/property_enumeration.h"
#include "../base/settings.h"
#include "../base/xcaf.h"
#include "app_module.h"
#include "theme.h"
#include "widget_model_tree.h"

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItemIterator>

#include <unordered_map>
#include <unordered_set>

namespace Mayo {

namespace {

template<size_t N>
QByteArray QByteArray_frowRawData(const char (&str)[N]) {
    return QByteArray::fromRawData(str, N);
}

} // namespace

class WidgetModelTreeBuilder_Xde::Module : public QObject, public PropertyGroup {
public:
    Module(ApplicationPtr app)
        : QObject(app.get()),
          PropertyGroup(app->settings()),
          instanceNameFormat(this, MAYO_TEXT_ID("Mayo::WidgetModelTreeBuilder_Xde", "instanceNameFormat"))
    {
        this->instanceNameFormat.setEnumeration(&enumInstanceNameFormat());
        this->setObjectName("WidgetModelTreeBuilder_Xde::Module");
        auto settings = app->settings();
        settings->addSetting(&this->instanceNameFormat, AppModule::get(app)->groupId_application);
        settings->addGroupResetFunction(AppModule::get(app)->groupId_application, [&]{
            this->instanceNameFormat.setValue(int(NameFormat::Product));
        });
    }

    static Module* get(ApplicationPtr app) {
        return app->findChild<Module*>("WidgetModelTreeBuilder_Xde::Module", Qt::FindDirectChildrenOnly);
    }

    enum class NameFormat { Instance, Product, Both };

    static const Enumeration& enumInstanceNameFormat()
    {
        static const Enumeration enumFormat = {
            { int(NameFormat::Instance), MAYO_TEXT_ID("Mayo::WidgetModelTreeBuilder_Xde", "nameInstance") },
            { int(NameFormat::Product),  MAYO_TEXT_ID("Mayo::WidgetModelTreeBuilder_Xde", "nameProduct") },
            { int(NameFormat::Both),     MAYO_TEXT_ID("Mayo::WidgetModelTreeBuilder_Xde", "nameBoth") }
        };
        return enumFormat;
    }

    static QByteArray toInstanceNameTemplate(NameFormat format)
    {
        static const char templateInstance[] = "%instance";
        static const char templateProduct[] = "%product";
        // UTF8 rightwards arrow : \xe2\x86\x92
        static const char templateBoth[] = "%instance \xe2\x86\x92 %product";
        switch (format) {
        case NameFormat::Instance: return QByteArray_frowRawData(templateInstance);
        case NameFormat::Product: return QByteArray_frowRawData(templateProduct);
        case NameFormat::Both: return QByteArray_frowRawData(templateBoth);
        }
        return QByteArray();
    }

    QByteArray instanceNameTemplate() const {
        return Module::toInstanceNameTemplate(this->instanceNameFormat.valueAs<Module::NameFormat>());
    }

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

    PropertyEnumeration instanceNameFormat;
};

bool WidgetModelTreeBuilder_Xde::supportsDocumentTreeNode(const DocumentTreeNode& node) const
{
    return XCaf::isShape(node.label());
}

void WidgetModelTreeBuilder_Xde::refreshTextTreeItem(
        const DocumentTreeNode& node, QTreeWidgetItem* treeItem)
{
    const TDF_Label labelNode = node.label();
    TDF_LabelSequence seqLabelRefresh;
    if (XCaf::isShapeReference(labelNode)
            && m_module->instanceNameTemplate().contains("%product"))
    {
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

// BEWARE Not thread-safe, should be called from main(GUI) thread
void WidgetModelTreeBuilder_Xde::registerApplication(ApplicationPtr app)
{
    m_module = Module::get(app);
    if (!m_module)
        m_module = new Module(app);
}

WidgetModelTree_UserActions WidgetModelTreeBuilder_Xde::createUserActions(QObject *parent)
{
    WidgetModelTree_UserActions userActions;
    auto group = new QActionGroup(parent);
    group->setExclusive(true);
    for (const Enumeration::Item& item : Module::enumInstanceNameFormat().items()) {
        const QString actionText =
                MAYO_TEXT_ID("Mayo::WidgetModelTreeBuilder_Xde", "Show %1").tr().arg(item.name.tr());
        auto action = new QAction(actionText, parent);
        action->setCheckable(true);
        action->setData(item.name.key);
        userActions.items.push_back(action);
        group->addAction(action);
    }

    QObject::connect(group, &QActionGroup::triggered, [=](QAction* action) {
        this->setInstanceNameFormat(action->data().toByteArray());
    });

    userActions.fnSyncItems = [=]{
        for (QAction* action : userActions.items) {
            if (action->data().toByteArray() == this->instanceNameFormat())
                action->setChecked(true);
        }
    };

    return userActions;
}

QTreeWidgetItem* WidgetModelTreeBuilder_Xde::guiCreateXdeTreeNode(
        QTreeWidgetItem* guiParentNode, const DocumentTreeNode& node)
{
    auto guiNode = new QTreeWidgetItem(guiParentNode);
    const QString stdName = CafUtils::labelAttrStdName(node.label());
    guiNode->setText(0, stdName);
    WidgetModelTree::setDocumentTreeNode(guiNode, node);
    const QIcon icon = Module::shapeIcon(node.label());
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
                const QIcon icon = Module::shapeIcon(nodeLabel);
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

QByteArray WidgetModelTreeBuilder_Xde::instanceNameFormat() const
{
    return m_module->instanceNameFormat.name();
}

void WidgetModelTreeBuilder_Xde::setInstanceNameFormat(const QByteArray& format)
{
    if (format == this->instanceNameFormat())
        return;

    m_module->instanceNameFormat.setValue(Module::enumInstanceNameFormat().findValue(format));
    for (QTreeWidgetItemIterator it(this->treeWidget()); *it; ++it) {
        if (WidgetModelTree::holdsDocumentTreeNode(*it))
            this->refreshXdeAssemblyNodeItemText(*it);
    }
}

std::unique_ptr<WidgetModelTreeBuilder> WidgetModelTreeBuilder_Xde::clone() const
{
    auto builder = std::make_unique<WidgetModelTreeBuilder_Xde>();
    builder->m_module = this->m_module;
    builder->m_isMergeXdeReferredShapeOn = this->m_isMergeXdeReferredShapeOn;
    return builder;
}

void WidgetModelTreeBuilder_Xde::refreshXdeAssemblyNodeItemText(QTreeWidgetItem* item)
{
    const DocumentTreeNode docTreeNode = WidgetModelTree::documentTreeNode(item);
    const TDF_Label label = docTreeNode.label();
    if (XCaf::isShapeReference(label)) {
        const TDF_Label& instanceLabel = label;
        const TDF_Label productLabel = XCaf::shapeReferred(instanceLabel);
        const QString itemText = this->referenceItemText(instanceLabel, productLabel);
        item->setText(0, itemText);
    }
    else {
        item->setText(0, CafUtils::labelAttrStdName(label));
    }
}

QString WidgetModelTreeBuilder_Xde::referenceItemText(
        const TDF_Label& instanceLabel, const TDF_Label& productLabel) const
{
    const QString instanceName = CafUtils::labelAttrStdName(instanceLabel).trimmed();
    const QString productName = CafUtils::labelAttrStdName(productLabel).trimmed();
    const auto format = m_module->instanceNameFormat.valueAs<Module::NameFormat>();
    const QByteArray strTemplate = Module::toInstanceNameTemplate(format);
    QString itemText = QString::fromUtf8(strTemplate);
    itemText.replace("%instance", instanceName)
            .replace("%product", productName);
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
