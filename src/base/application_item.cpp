/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application_item.h"

namespace Mayo {

ApplicationItem::ApplicationItem(const DocumentPtr& doc)
    : m_doc(doc),
      m_docTreeNode(DocumentTreeNode::null())
{ }

ApplicationItem::ApplicationItem(const DocumentTreeNode& node)
    : m_docTreeNode(node)
{ }

bool ApplicationItem::isValid() const
{
    return this->isDocument() || this->isDocumentTreeNode();
}

bool ApplicationItem::isDocument() const
{
    return !m_doc.IsNull();
}

bool ApplicationItem::isDocumentTreeNode() const
{
    return m_docTreeNode.isValid();
}

DocumentPtr ApplicationItem::document() const
{
    if (this->isDocument())
        return m_doc;
    else if (this->isDocumentTreeNode())
        return m_docTreeNode.document();

    return DocumentPtr();
}

const DocumentTreeNode& ApplicationItem::documentTreeNode() const
{
    return this->isDocumentTreeNode() ? m_docTreeNode : DocumentTreeNode::null();
}

bool ApplicationItem::operator==(const ApplicationItem& other) const
{
    return m_doc == other.m_doc
            && m_docTreeNode.document() == other.m_docTreeNode.document()
            && m_docTreeNode.id() == other.m_docTreeNode.id();
}

} // namespace Mayo
