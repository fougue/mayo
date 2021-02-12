/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "caf_utils.h"
#include "string_utils.h"

#include <TDataStd_Name.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_Tool.hxx>
#include <XCAFApp_Application.hxx>

namespace Mayo {

QLatin1String CafUtils::labelTag(const TDF_Label& label)
{
    static thread_local TCollection_AsciiString entry;
    TDF_Tool::Entry(label, entry);
    return QLatin1String(entry.ToCString(), entry.Length());
}

QString CafUtils::labelAttrStdName(const TDF_Label& label)
{
    Handle_TDataStd_Name attrName;
    if (label.FindAttribute(TDataStd_Name::GetID(), attrName))
        return StringUtils::fromUtf16(attrName->Get());
    else
        return QString();
}

void CafUtils::setLabelAttrStdName(const TDF_Label& label, const QString& name)
{
    TDataStd_Name::Set(label, StringUtils::toUtf16<TCollection_ExtendedString>(name));
}

bool CafUtils::isNullOrEmpty(const TDF_Label& label)
{
    if (label.IsNull())
        return true;

    if (!label.HasAttribute())
        return true;

    return false;
}

bool CafUtils::hasAttribute(const TDF_Label& label, const Standard_GUID& attrGuid)
{
    Handle_TDF_Attribute attr;
    return label.FindAttribute(attrGuid, attr);
}

} // namespace Mayo
