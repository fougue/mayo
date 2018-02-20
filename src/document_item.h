/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#pragma once

#include "property_builtins.h"
#include <cstring>
#include <vector>

namespace Mayo {

class Document;

class DocumentItem : public PropertyOwner
{
public:
    DocumentItem();
    virtual ~DocumentItem();

    Document* document();
    const Document* document() const;
    void setDocument(Document* doc);

    PropertyQString propertyLabel;

    const std::vector<DocumentItem*>& outItems() const; // For future use

    virtual const char* dynType() const = 0;

protected:
    void onPropertyChanged(Property* prop) override;
    void addChildItem(DocumentItem* item);
    void setParentItem(DocumentItem* parentItem);

private:
    Document* m_document = nullptr;
    std::vector<DocumentItem*> m_vecOutItem;
};

class PartItem : public DocumentItem
{
public:
    PartItem();

    virtual bool isNull() const;

    static const char* type;
    const char* dynType() const override;

    PropertyDouble propertyVolume; // Read-only
    PropertyDouble propertyArea; // Read-only
};

bool sameType(const DocumentItem* lhs, const DocumentItem* rhs);

template<typename T>
bool sameType(const DocumentItem* item)
{
    return item != nullptr ?
                std::strcmp(item->dynType(), T::type) == 0 :
                false;
}

} // namespace Mayo
