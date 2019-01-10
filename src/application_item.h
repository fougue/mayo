/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "xde_document_item.h"

namespace Mayo {

class ApplicationItem {
public:
    ApplicationItem() = default;
    ApplicationItem(Document* doc);
    ApplicationItem(DocumentItem* docItem);
    ApplicationItem(const XdeAssemblyNode& lbl);

    bool isValid() const;
    bool isDocument() const;
    bool isDocumentItem() const;
    bool isXdeAssemblyNode() const;

    Document* document() const;
    DocumentItem* documentItem() const;
    const XdeAssemblyNode& xdeAssemblyNode() const;

    bool operator==(const ApplicationItem& other) const;

private:
    Document* m_doc;
    DocumentItem* m_docItem;
    XdeAssemblyNode m_xdeAsmTreeNode;
};

} // namespace Mayo
