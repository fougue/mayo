/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "../base/property_builtins.h"
#include "../base/document.h"

namespace Mayo {

// Provides relevant properties for a Document object
// TODO Connect to Document "changed" signals to update the properties
class DocumentPropertyGroup : public PropertyGroup {
    MAYO_DECLARE_TEXT_ID_FUNCTIONS(Mayo::DocumentPropertyGroup)
public:
    DocumentPropertyGroup(const DocumentPtr& doc);

    PropertyFilePath filePath{ this, textId("filepath") };
    PropertyString strFileSize{ this, textId("fileSize") };
    PropertyString strCreatedDateTime{ this, textId("createdDateTime") };
    PropertyString strModifiedDateTime{ this, textId("modifiedDateTime") };
    PropertyString strOwner{ this, textId("owner") };
    PropertyInt entityCount{ this, textId("entityCount") };
};

} // namespace Mayo
