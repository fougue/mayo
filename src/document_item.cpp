#include "document_item.h"

namespace Mayo {

Document *DocumentItem::document()
{
    return m_document;
}

const Document *DocumentItem::document() const
{
    return m_document;
}

void DocumentItem::setDocument(Document *doc)
{
    m_document = doc;
}

const QString& DocumentItem::label() const
{
    return m_label;
}

void DocumentItem::setLabel(const QString &v)
{
    m_label = v;
}

const std::vector<DocumentItem*>& DocumentItem::outItems() const
{
    return m_outItems;
}

const QString& PartItem::filePath() const
{
    return m_filePath;
}

void PartItem::setFilePath(const QString &v)
{
    m_filePath = v;
}

bool PartItem::isNull() const
{
    return false;
}

const char* PartItem::type = "247d246a-6316-4a99-b68e-bdcf565fa8aa";
const char* PartItem::dynType() const { return PartItem::type; }

bool sameType(const DocumentItem *lhs, const DocumentItem *rhs)
{
    if (lhs != nullptr && rhs != nullptr)
        return std::strcmp(lhs->dynType(), rhs->dynType()) == 0;
    return false;
}

} // namespace Mayo
