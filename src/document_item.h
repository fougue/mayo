/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "property_builtins.h"
#include <cstring>
#include <vector>

namespace Mayo {

class Document;

class DocumentItem : public PropertyOwner {
public:
    DocumentItem();
    virtual ~DocumentItem();

    Document* document();
    const Document* document() const;
    void setDocument(Document* doc);

    PropertyQString propertyLabel;

    const std::vector<DocumentItem*>& outItems() const; // For future use

    virtual const char* dynTypeName() const = 0;

protected:
    void onPropertyChanged(Property* prop) override;
    void addChildItem(DocumentItem* item);
    void setParentItem(DocumentItem* parentItem);

private:
    Document* m_document = nullptr;
    std::vector<DocumentItem*> m_vecOutItem;
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
    return item != nullptr ?
                std::strcmp(item->dynTypeName(), T::TypeName) == 0 :
                false;
}

} // namespace Mayo
