/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_item.h"

#include "document.h"

#include <QtCore/QCoreApplication>
#include <cassert>
#include <limits>

namespace Mayo {

DocumentItem::DocumentItem()
    : propertyLabel(this, QCoreApplication::translate("Mayo::DocumentItem", "Label"))
{
}

DocumentItem::~DocumentItem()
{
}

Document* DocumentItem::document() const
{
    return m_document;
}

void DocumentItem::setDocument(Document* doc)
{
    m_document = doc;
}

std::unique_ptr<PropertyOwnerSignals> DocumentItem::propertiesAtNode(TreeNodeId) const
{
    return std::unique_ptr<PropertyOwnerSignals>();
}

void DocumentItem::onPropertyChanged(Property* prop)
{
    if (m_document)
        emit m_document->itemPropertyChanged(this, prop);
}

PartItem::PartItem()
    : propertyArea(
          this, QCoreApplication::translate("Mayo::PartItem", "Computed area")),
      propertyVolume(
          this, QCoreApplication::translate("Mayo::PartItem", "Computed volume"))
{
//    this->propertyVolume.setRange(0., std::numeric_limits<double>::max());
//    this->propertyArea.setRange(0., std::numeric_limits<double>::max());
    this->propertyVolume.setUserReadOnly(true);
    this->propertyArea.setUserReadOnly(true);
}

bool PartItem::isNull() const
{
    return false;
}

const char PartItem::TypeName[] = "247d246a-6316-4a99-b68e-bdcf565fa8aa";
const char* PartItem::dynTypeName() const { return PartItem::TypeName; }

bool sameType(const DocumentItem* lhs, const DocumentItem* rhs)
{
    if (lhs && rhs)
        return std::strcmp(lhs->dynTypeName(), rhs->dynTypeName()) == 0;

    return false;
}

DocumentItemNode::DocumentItemNode(DocumentItem* docItem, TreeNodeId nodeId)
    : documentItem(docItem),
      id(nodeId)
{
}

bool DocumentItemNode::isValid() const
{
    return this->documentItem && this->id != 0;
}

const DocumentItemNode& DocumentItemNode::null()
{
    static const DocumentItemNode node = {};
    return node;
}

} // namespace Mayo
