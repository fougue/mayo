/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "document_property_group.h"

#include "app_module.h"
#include "qstring_utils.h"
#include "../qtcommon/filepath_conv.h"
#include "../qtcommon/qstring_conv.h"

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

namespace Mayo {

DocumentPropertyGroup::DocumentPropertyGroup(const DocumentPtr& doc)
{
    this->filePath.setValue(filepathCanonical(doc->filePath()));

    const auto fileSize = filepathFileSize(doc->filePath());
    auto appModule = AppModule::get();
    const QString qstrFileSize = QStringUtils::bytesText(fileSize, appModule->qtLocale());
    this->strFileSize.setValue(to_stdString(qstrFileSize));

    const QFileInfo fileInfo = filepathTo<QFileInfo>(doc->filePath());
    const QString strCreated = appModule->qtLocale().toString(fileInfo.birthTime(), QLocale::ShortFormat);
    const QString strModified = appModule->qtLocale().toString(fileInfo.lastModified(), QLocale::ShortFormat);
    this->strCreatedDateTime.setValue(to_stdString(strCreated));
    this->strModifiedDateTime.setValue(to_stdString(strModified));

    this->strOwner.setValue(to_stdString(fileInfo.owner()));

    this->entityCount.setValue(doc->entityCount());

    for (Property* prop : this->properties())
        prop->setUserReadOnly(true);
}

} // namespace Mayo
