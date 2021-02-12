/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TDF_Label.hxx>
#include <QtCore/QString>
#include <QtCore/QHash>

namespace Mayo {

struct CafUtils {
    static QLatin1String labelTag(const TDF_Label& label);

    static QString labelAttrStdName(const TDF_Label& label);
    static void setLabelAttrStdName(const TDF_Label& label, const QString& name);

    static bool isNullOrEmpty(const TDF_Label& label);

    template<typename TDF_ATTRIBUTE>
    static opencascade::handle<TDF_ATTRIBUTE> findAttribute(const TDF_Label& label);

    static bool hasAttribute(const TDF_Label& label, const Standard_GUID& attrGuid);
    template<typename TDF_ATTRIBUTE> static bool hasAttribute(const TDF_Label& label);
};

} // namespace Mayo

#include <TDF_LabelMapHasher.hxx>
namespace std {

//! Specialization of C++11 std::hash<> functor for TDF_Label
template<> struct hash<TDF_Label> {
    inline size_t operator()(const TDF_Label& lbl) const {
        return TDF_LabelMapHasher::HashCode(lbl, INT_MAX);
        // return qHash(Mayo::CafUtils::labelTag(lbl));
    }
};

} // namespace std

// --
// -- Implementation
// --

namespace Mayo {

template<typename TDF_ATTRIBUTE>
opencascade::handle<TDF_ATTRIBUTE> CafUtils::findAttribute(const TDF_Label& label)
{
    opencascade::handle<TDF_ATTRIBUTE> attr;
    label.FindAttribute(TDF_ATTRIBUTE::GetID(), attr);
    return attr;
}

template<typename TDF_ATTRIBUTE>
bool CafUtils::hasAttribute(const TDF_Label& label)
{
    return hasAttribute(label, TDF_ATTRIBUTE::GetID());
}

} // namespace Mayo

