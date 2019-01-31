/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "gpx_document_item_factory.h"
#include "document_item.h"

#include <algorithm>

namespace Mayo {

GpxDocumentItemFactory *GpxDocumentItemFactory::instance()
{
    static GpxDocumentItemFactory global;
    return &global;
}

GpxDocumentItem* GpxDocumentItemFactory::create(DocumentItem* docItem)
{
    const char* strDocItemTypeName = docItem->dynTypeName();
    for (const CreatorData& creatorData : m_vecCreatorData) {
        if (creatorData.strDocumentItemTypeName == strDocItemTypeName)
            return creatorData.func(docItem);
    }
    return nullptr;
}

void GpxDocumentItemFactory::registerCreatorFunction(
        const char* strDocumentItemTypeName, const CreatorFunction& func)
{
    auto itFound = std::find_if(
                m_vecCreatorData.cbegin(),
                m_vecCreatorData.cend(),
                [=](const CreatorData& candidate) {
        return candidate.strDocumentItemTypeName == strDocumentItemTypeName;
    });
    if (itFound == m_vecCreatorData.cend())
        m_vecCreatorData.push_back({ strDocumentItemTypeName, func });
}

} // namespace Mayo
