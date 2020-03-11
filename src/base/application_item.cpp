/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application_item.h"

namespace Mayo {

ApplicationItem::ApplicationItem(Document *doc)
    : m_doc(doc),
      m_docItem(nullptr),
      m_docItemNode(DocumentItemNode::null())
{ }

ApplicationItem::ApplicationItem(DocumentItem *docItem)
    : m_doc(nullptr),
      m_docItem(docItem),
      m_docItemNode(DocumentItemNode::null())
{ }

ApplicationItem::ApplicationItem(const DocumentItemNode &node)
    : m_doc(nullptr),
      m_docItem(nullptr),
      m_docItemNode(node)
{ }

bool ApplicationItem::isValid() const {
    return this->isDocument()
            || this->isDocumentItem()
            || this->isDocumentItemNode();
}

bool ApplicationItem::isDocument() const {
    return m_doc != nullptr;
}

bool ApplicationItem::isDocumentItem() const {
    return m_docItem != nullptr;
}

bool ApplicationItem::isDocumentItemNode() const {
    return m_docItemNode.isValid();
}

Document* ApplicationItem::document() const
{
    if (this->isDocument())
        return m_doc;
    else if (this->isDocumentItem())
        return m_docItem->document();
    else if (this->isDocumentItemNode())
        return m_docItemNode.documentItem->document();

    return nullptr;
}

DocumentItem* ApplicationItem::documentItem() const
{
    if (this->isDocumentItem())
        return m_docItem;
    else if (this->isDocumentItemNode())
        return m_docItemNode.documentItem;

    return nullptr;
}

const DocumentItemNode& ApplicationItem::documentItemNode() const
{
    return this->isDocumentItemNode() ?
                m_docItemNode : DocumentItemNode::null();
}

bool ApplicationItem::operator==(const ApplicationItem &other) const
{
    return m_doc == other.m_doc
            && m_docItem == other.m_docItem
            && m_docItemNode.documentItem == other.m_docItemNode.documentItem
            && m_docItemNode.id == other.m_docItemNode.id;
}

} // namespace Mayo
