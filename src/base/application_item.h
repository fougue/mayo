/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
