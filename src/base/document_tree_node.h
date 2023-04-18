/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "libtree.h"

namespace Mayo {

// Provides a convenient item for model tree nodes within a Document object
class DocumentTreeNode {
public:
    DocumentTreeNode() = default;
    DocumentTreeNode(const DocumentPtr& docPtr, TreeNodeId nodeId);

    bool isValid() const;
    static const DocumentTreeNode& null();

    TDF_Label label() const;
    bool isEntity() const;
    bool isLeaf() const;

    const DocumentPtr& document() const { return m_document; }
    TreeNodeId id() const { return m_id; }

    bool operator==(const DocumentTreeNode& other) const;

private:
    DocumentPtr m_document; // TODO Document* or Document::identifier instead ?
    TreeNodeId m_id = 0;
};

} // namespace Mayo
