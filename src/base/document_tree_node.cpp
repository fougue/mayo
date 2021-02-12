/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
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
        return m_document->modelTree().nodeData(m_id);
    else
        return TDF_Label();
}

bool DocumentTreeNode::isEntity() const
{
    return this->isValid() ? m_document->isEntity(m_id) : false;
}

bool DocumentTreeNode::operator==(const DocumentTreeNode& other) const
{
    if (!this->isValid() || !other.isValid())
        return false;

    return m_document->identifier() == other.document()->identifier()
            && m_id == other.id();
}

} // namespace Mayo
