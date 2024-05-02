/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "caf_utils.h"

#include <TDataStd_Name.hxx>
#include <TDF_Tool.hxx>

namespace Mayo {

const TCollection_AsciiString& CafUtils::labelTag(const TDF_Label& label)
{
    // Note: thread_local implies "static"
    //       See https://en.cppreference.com/w/cpp/language/storage_duration
    thread_local TCollection_AsciiString entry;
    TDF_Tool::Entry(label, entry);
    return entry;
}

const TCollection_ExtendedString& CafUtils::labelAttrStdName(const TDF_Label& label)
{
    OccHandle<TDataStd_Name> attrName;
    if (label.FindAttribute(TDataStd_Name::GetID(), attrName)) {
        return attrName->Get();
    }
    else {
        static const TCollection_ExtendedString nullStr = {};
        return nullStr;
    }
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
    OccHandle<TDF_Attribute> attr;
    return label.FindAttribute(attrGuid, attr);
}

TDF_LabelSequence CafUtils::makeLabelSequence(std::initializer_list<TDF_Label> listLabel)
{
    TDF_LabelSequence seqLabel;
    for (const TDF_Label& label : listLabel)
        seqLabel.Append(label);

    return seqLabel;
}

} // namespace Mayo
