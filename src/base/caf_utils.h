/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "occ_handle.h"
#include <Standard_Version.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>

#include <vector>

class TCollection_AsciiString;
class TCollection_ExtendedString;
class TDataStd_NamedData;

namespace Mayo {

// Provides helper functions for OpenCascade CAF related libraries
struct CafUtils {

    // -- TDF_Label

    // Returns a string representation of tag list path to 'label'
    static const TCollection_AsciiString& labelTag(const TDF_Label& label);

    // Returns the name attribute(if any) attached to 'label'
    // Empty string is returned if no name attribute
    static const TCollection_ExtendedString& labelAttrStdName(const TDF_Label& label);

    // Is 'label' null or empty(ie no attributes)?
    static bool isNullOrEmpty(const TDF_Label& label);

    // Returns attribute of type 'AttributeType'(result may be null)?
    template<typename AttributeType>
    static OccHandle<AttributeType> findAttribute(const TDF_Label& label);

    // Is there an attribute of identifier 'attrGuid' attached to 'label'?
    static bool hasAttribute(const TDF_Label& label, const Standard_GUID& attrGuid);

    // Is there an attribute of type 'AttributeType' attached to 'label'?
    template<typename AttributeType> static bool hasAttribute(const TDF_Label& label);

    // Returns a TDF_LabelSequence object built from initializer list
    static TDF_LabelSequence makeLabelSequence(std::initializer_list<TDF_Label> listLabel);

    // -- TDataStd_NamedData

    // Returns the count of all data contained in TDataStd_NamedData objecy
    static int namedDataCount(const OccHandle<TDataStd_NamedData>& data);

    // Various data types that can be stored in TDataStd_NamedData
    enum class NamedDataType {
        None, Int, Double, String, Byte, IntArray, DoubleArray
    };
    // Key definition for the data contained in TDataStd_NamedData
    // This holds the name(label) and type of the associated data
    struct NamedDataKey {
        const TCollection_ExtendedString* ptrLabel = nullptr;
        NamedDataType type = NamedDataType::None;
        const TCollection_ExtendedString& label() const { return *this->ptrLabel; }
    };
    // Returns all the keys of the data stored in TDataStd_NamedData object
    static std::vector<NamedDataKey> getNamedDataKeys(const OccHandle<TDataStd_NamedData>& data);
};

} // namespace Mayo

#if OCC_VERSION_HEX < 0x070800
#include <TDF_LabelMapHasher.hxx>
namespace std {

// Specialization of C++11 std::hash<> functor for TDF_Label
template<> struct hash<TDF_Label> {
    inline size_t operator()(const TDF_Label& lbl) const {
        return TDF_LabelMapHasher::HashCode(lbl, INT_MAX);
    }
};

} // namespace std
#endif

// --
// -- Implementation
// --

namespace Mayo {

template<typename AttributeType>
OccHandle<AttributeType> CafUtils::findAttribute(const TDF_Label& label)
{
    OccHandle<AttributeType> attr;
    label.FindAttribute(AttributeType::GetID(), attr);
    return attr;
}

template<typename AttributeType>
bool CafUtils::hasAttribute(const TDF_Label& label)
{
    return hasAttribute(label, AttributeType::GetID());
}

} // namespace Mayo

