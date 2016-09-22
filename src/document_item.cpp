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

#include "document_item.h"

#include "document.h"
#include <QtCore/QCoreApplication>

namespace Mayo {

DocumentItem::DocumentItem()
    : propertyLabel(
          this, QCoreApplication::translate("Mayo::DocumentItem", "label"))
{
}

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

const std::vector<DocumentItem*>& DocumentItem::outItems() const
{
    return m_outItems;
}

void DocumentItem::onPropertyChanged(Property *prop)
{
    if (m_document != nullptr)
        emit m_document->itemPropertyChanged(this, prop);
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
