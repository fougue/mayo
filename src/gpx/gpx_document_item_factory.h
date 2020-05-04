/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#if 0
#pragma once

#include <functional>
#include <vector>

namespace Mayo {

class DocumentItem;
class GpxDocumentItem;

// Singleton providing creation of GpxDocumentItem objects suited to display
// input DocumentItem objects
class GpxDocumentItemFactory {
public:
    static GpxDocumentItemFactory* instance();

    GpxDocumentItem* create(DocumentItem* docItem);

    using CreatorFunction = std::function<GpxDocumentItem* (DocumentItem*)>;
    void registerCreatorFunction(const char* strDocumentItemTypeName, const CreatorFunction& func);

    // Helper for GpxDocumentItem creator function
    template<typename DOC_ITEM, typename GPX_DOC_ITEM>
    static GPX_DOC_ITEM* createGpx(DocumentItem* docItem) {
        return new GPX_DOC_ITEM(dynamic_cast<DOC_ITEM*>(docItem));
    }

private:
    GpxDocumentItemFactory() = default;

    struct CreatorData {
        const char* strDocumentItemTypeName;
        CreatorFunction func;
    };

    std::vector<CreatorData> m_vecCreatorData;
};

} // namespace Mayo
#endif
