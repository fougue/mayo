/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application_item.h"

namespace Mayo {

ApplicationItem::ApplicationItem(Document *doc)
    : m_doc(doc),
      m_docItem(nullptr),
      m_xdeAsmTreeNode(XdeAssemblyNode::null())
{ }

ApplicationItem::ApplicationItem(DocumentItem *docItem)
    : m_doc(nullptr),
      m_docItem(docItem),
      m_xdeAsmTreeNode(XdeAssemblyNode::null())
{ }

ApplicationItem::ApplicationItem(const XdeAssemblyNode &node)
    : m_doc(nullptr),
      m_docItem(nullptr),
      m_xdeAsmTreeNode(node)
{ }

bool ApplicationItem::isValid() const {
    return this->isDocument()
            || this->isDocumentItem()
            || this->isXdeAssemblyNode();
}

bool ApplicationItem::isDocument() const {
    return m_doc != nullptr;
}

bool ApplicationItem::isDocumentItem() const {
    return m_docItem != nullptr;
}

bool ApplicationItem::isXdeAssemblyNode() const {
    return m_xdeAsmTreeNode.isValid();
}

Document *ApplicationItem::document() const
{
    if (this->isDocument())
        return m_doc;
    else if (this->isDocumentItem())
        return m_docItem->document();
    else if (this->isXdeAssemblyNode())
        return m_xdeAsmTreeNode.ownerDocItem->document();
    return nullptr;
}

DocumentItem *ApplicationItem::documentItem() const
{
    if (this->isDocumentItem())
        return m_docItem;
    else if (this->isXdeAssemblyNode())
        return m_xdeAsmTreeNode.ownerDocItem;
    return nullptr;
}

const XdeAssemblyNode& ApplicationItem::xdeAssemblyNode() const
{
    return this->isXdeAssemblyNode() ?
                m_xdeAsmTreeNode : XdeAssemblyNode::null();
}

bool ApplicationItem::operator==(const ApplicationItem &other) const
{
    return m_doc == other.m_doc
            && m_docItem == other.m_docItem
            && m_xdeAsmTreeNode.ownerDocItem == other.m_xdeAsmTreeNode.ownerDocItem
            && m_xdeAsmTreeNode.nodeId == other.m_xdeAsmTreeNode.nodeId;
}

} // namespace Mayo
