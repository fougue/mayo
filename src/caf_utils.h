/****************************************************************************
** Copyright (c) 2019, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <QtCore/QString>
#include <QtCore/QHash>

namespace Mayo {

struct CafUtils {
    static QLatin1String labelTag(const TDF_Label& label);
    static QString labelAttrStdName(const TDF_Label& label);

    static Handle_TDocStd_Document createXdeDocument(const char* format = "XmlXCAF");
};

} // namespace Mayo

namespace std {

//! Specialization of C++11 std::hash<> functor for TDF_Label
template<> struct hash<TDF_Label> {
    inline size_t operator()(const TDF_Label& lbl) const
    { return qHash(Mayo::CafUtils::labelTag(lbl)); }
};

} // namespace std
