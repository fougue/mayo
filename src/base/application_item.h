/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document.h"
#include "document_tree_node.h"

namespace Mayo {

// Provides a common item that could be either a Document or some model tree node within a Document
class ApplicationItem {
public:
    ApplicationItem() = default;
    ApplicationItem(const DocumentPtr& doc);
    ApplicationItem(const DocumentTreeNode& node);

    bool isValid() const;
    bool isDocument() const;
    bool isDocumentTreeNode() const;

    DocumentPtr document() const;
    const DocumentTreeNode& documentTreeNode() const;

    bool operator==(const ApplicationItem& other) const;

private:
    DocumentPtr m_doc;
    DocumentTreeNode m_docTreeNode;
};

} // namespace Mayo
