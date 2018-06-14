/****************************************************************************
** Copyright (c) 2018, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "caf_utils.h"

#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>
#include <XCAFApp_Application.hxx>

#include "fougtools/occtools/qt_utils.h"

#include <mutex>

namespace occ {

namespace Internal {
static std::mutex mutex_XCAFApplication;
} // namespace Internal

QString CafUtils::labelTag(const TDF_Label &label)
{
    TCollection_AsciiString entry;
    TDF_Tool::Entry(label, entry);
    return QString::fromUtf8(entry.ToCString());
}

QString CafUtils::labelAttrStdName(const TDF_Label &label)
{
    Handle_TDataStd_Name attrName;
    if (label.FindAttribute(TDataStd_Name::GetID(), attrName))
        return occ::QtUtils::toQString(attrName->Get());
    return QString();
}

Handle_TDocStd_Document CafUtils::createXdeDocument(const char *format)
{
    Handle_TDocStd_Document doc;
    std::lock_guard<std::mutex> lock(Internal::mutex_XCAFApplication);
    XCAFApp_Application::GetApplication()->NewDocument(format, doc);
    return doc;
}

} // namespace occ
