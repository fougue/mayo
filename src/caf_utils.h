#pragma once

#include <TDF_Label.hxx>
#include <TDocStd_Document.hxx>
#include <QtCore/QString>
#include <QtCore/QHash>

namespace occ {

class CafUtils {
public:
    static QString labelTag(const TDF_Label &label);
    static QString labelAttrStdName(const TDF_Label &label);

    static Handle_TDocStd_Document createXdeDocument(const char* format = "XmlXCAF");
};

} // namespace occ

namespace std {

//! Specialization of C++11 std::hash<> functor for TDF_Label
template<> struct hash<TDF_Label> {
    inline std::size_t operator()(const TDF_Label& lbl) const
    { return qHash(occ::CafUtils::labelTag(lbl)); }
};

} // namespace std
