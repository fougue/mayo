/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_item.h"

namespace Mayo {

class ApplicationItem {
public:
    ApplicationItem() = default;
    ApplicationItem(Document* doc);
    ApplicationItem(DocumentItem* docItem);
    ApplicationItem(const DocumentItemNode& node);

    bool isValid() const;
    bool isDocument() const;
    bool isDocumentItem() const;
    bool isDocumentItemNode() const;

    Document* document() const;
    DocumentItem* documentItem() const;
    const DocumentItemNode& documentItemNode() const;

    bool operator==(const ApplicationItem& other) const;

private:
    Document* m_doc;
    DocumentItem* m_docItem;
    DocumentItemNode m_docItemNode;
};

} // namespace Mayo
