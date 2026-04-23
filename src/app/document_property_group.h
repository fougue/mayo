/****************************************************************************
** Copyright (c) 2016, Fougue SAS <https://www.fougue.pro>
** SPDX-License-Identifier: BSD-2-Clause
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
