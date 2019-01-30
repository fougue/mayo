/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "application.h"
#include "document.h"
#include "document_item.h"

namespace Mayo {

Document::Document(QObject* parent)
    : PropertyOwnerSignals(parent),
      propertyLabel(this, tr("Label")),
      propertyFilePath(this, tr("File path"))
{
    this->propertyFilePath.setUserReadOnly(true);
}

Document::~Document()
{
    for (DocumentItem* item : m_rootItems)
        delete item;
}

const QString &Document::label() const
{
    return this->propertyLabel.value();
}

void Document::setLabel(const QString &v)
{
    this->propertyLabel.setValue(v);
}

const QString &Document::filePath() const
{
    return this->propertyFilePath.value();
}

void Document::setFilePath(const QString &filepath)
{
    this->propertyFilePath.setValue(filepath);
}

bool Document::eraseRootItem(DocumentItem *docItem)
{
    auto itFound = std::find(m_rootItems.cbegin(), m_rootItems.cend(), docItem);
    if (itFound != m_rootItems.cend()) {
        m_rootItems.erase(itFound);
        emit itemErased(docItem);
        delete docItem;
        return true;
    }
    return false;
}

Span<DocumentItem* const> Document::rootItems() const
{
    return m_rootItems;
}

bool Document::isEmpty() const
{
    return m_rootItems.empty();
}

const char Document::TypeName[] = "Mayo::Document";
const char* Document::dynTypeName() const
{
    return Document::TypeName;
}

void Document::addRootItem(DocumentItem* item)
{
    item->setDocument(this);
    m_rootItems.push_back(item);
    emit itemAdded(item);
}

} // namespace Mayo
