/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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

    return {};
}

const DocumentTreeNode& ApplicationItem::documentTreeNode() const
{
    return this->isDocumentTreeNode() ? m_docTreeNode : DocumentTreeNode::null();
}

bool operator==(const ApplicationItem& lhs, const ApplicationItem& rhs)
{
    return lhs.document() == rhs.document()
           && lhs.documentTreeNode() == rhs.documentTreeNode();
}

} // namespace Mayo
