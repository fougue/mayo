/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
****************************************************************************/

#include "document_tree_node.h"
#include "document.h"

namespace Mayo {

DocumentTreeNode::DocumentTreeNode(const DocumentPtr& docPtr, TreeNodeId nodeId)
    : m_document(docPtr), m_id(nodeId)
{ }

bool DocumentTreeNode::isValid() const
{
    return !m_document.IsNull() && m_id != 0;
}

const DocumentTreeNode& DocumentTreeNode::null()
{
    static const DocumentTreeNode node = {};
    return node;
}

TDF_Label DocumentTreeNode::label() const
{
    if (this->isValid())
        return m_document->modelTreeNodeLabel(m_id);
    else
        return {};
}

bool DocumentTreeNode::isEntity() const
{
    return this->isValid() ? m_document->isEntity(m_id) : false;
}

bool DocumentTreeNode::isLeaf() const
{
    if (this->isValid())
        return m_document->modelTree().nodeIsLeaf(m_id);
    else
        return false;
}

bool operator==(const DocumentTreeNode& lhs, const DocumentTreeNode& rhs)
{
    const bool lhsValid = lhs.isValid();
    const bool rhsValid = rhs.isValid();
    if (!lhsValid || !rhsValid)
        return lhsValid == rhsValid;

    return lhs.document()->identifier() == rhs.document()->identifier()
           && lhs.id() == rhs.id();
}

} // namespace Mayo
