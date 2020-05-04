/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#if 0
#pragma once

#include "property_builtins.h"
#include "libtree.h"
#include <cstring>
#include <memory>
#include <vector>

namespace Mayo {

class Document;

class DocumentItem : public PropertyOwner {
public:
    DocumentItem();
    virtual ~DocumentItem();

    Document* document() const;
    void setDocument(Document* doc);

    PropertyQString propertyLabel;

    virtual std::unique_ptr<PropertyOwnerSignals> propertiesAtNode(TreeNodeId nodeId) const;

    virtual const char* dynTypeName() const = 0;

protected:
    void onPropertyChanged(Property* prop) override;

private:
    Document* m_document = nullptr;
};

struct DocumentItemNode {
    DocumentItemNode() = default;
    DocumentItemNode(DocumentItem* docItem, TreeNodeId nodeId);

    bool isValid() const;
    static const DocumentItemNode& null();

    DocumentItem* documentItem;
    TreeNodeId id;
};

class PartItem : public DocumentItem {
public:
    PartItem();

    virtual bool isNull() const;

    static const char TypeName[];
    const char* dynTypeName() const override;

    PropertyArea propertyArea; // Read-only
    PropertyVolume propertyVolume; // Read-only
};

bool sameType(const DocumentItem* lhs, const DocumentItem* rhs);

template<typename T> bool sameType(const DocumentItem* item)
{
    return item ? std::strcmp(item->dynTypeName(), T::TypeName) == 0 : false;
}

} // namespace Mayo
#endif
