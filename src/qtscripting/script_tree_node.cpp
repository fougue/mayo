/****************************************************************************
** Copyright (c) 2024, Fougue Ltd. <https://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "script_tree_node.h"

#include "../base/caf_utils.h"
#include "../base/document.h"
#include "../qtcommon/qstring_conv.h"
#include "script_shape.h"

#include <QtCore/QVariant>

namespace Mayo {

ScriptTreeNode::ScriptTreeNode(const DocumentPtr& doc, TreeNodeId nodeId)
    : m_doc(doc),
    m_nodeId(nodeId)
{
}

unsigned ScriptTreeNode::parentId() const
{
    return m_doc ? m_doc->modelTree().nodeParent(m_nodeId) : 0;
}

QString ScriptTreeNode::tag() const
{
    return m_doc ? to_QString(CafUtils::labelTag(DocumentTreeNode::label(m_doc, m_nodeId))) : QString{};
}

QString ScriptTreeNode::name() const
{
    return m_doc ? to_QString(CafUtils::labelAttrStdName(DocumentTreeNode::label(m_doc, m_nodeId))) : QString{};
}

bool ScriptTreeNode::isAssembly() const
{
    return m_doc ? XCaf::isShapeAssembly(DocumentTreeNode::label(m_doc, m_nodeId)) : false;
}

bool ScriptTreeNode::isInstance() const
{
    return m_doc ? XCaf::isShapeReference(DocumentTreeNode::label(m_doc, m_nodeId)) : false;
}

bool ScriptTreeNode::isProduct() const
{
    return m_doc ? XCaf::isShapeSimple(DocumentTreeNode::label(m_doc, m_nodeId)) : false;
}

bool ScriptTreeNode::isComponent() const
{
    return m_doc ? XCaf::isShapeComponent(DocumentTreeNode::label(m_doc, m_nodeId)) : false;
}

bool ScriptTreeNode::hasSubShapes() const
{
    return m_doc ? XCaf::hasShapeSubs(DocumentTreeNode::label(m_doc, m_nodeId)) : false;
}

QStringList ScriptTreeNode::subShapeTags() const
{
    QStringList listTag;
    const TDF_LabelSequence seqShapeSubLabel = XCaf::shapeSubs(DocumentTreeNode::label(m_doc, m_nodeId));
    for (const TDF_Label& label : seqShapeSubLabel)
        listTag.push_back(to_QString(CafUtils::labelTag(label)));

    return listTag;
}

QVariant ScriptTreeNode::shape() const
{
    const TopoDS_Shape shape = XCaf::shape(DocumentTreeNode::label(m_doc, m_nodeId));
    return QVariant::fromValue(ScriptShape(shape));
}

} // namespace Mayo

