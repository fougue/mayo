/****************************************************************************
**  FougTools
**  Copyright Fougue (30 Mar. 2015)
**  contact@fougue.pro
**
** This software is a computer program whose purpose is to provide utility
** tools for the C++ language and the Qt toolkit.
**
** This software is governed by the CeCILL-C license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-C
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
****************************************************************************/

#pragma once

#include <QtCore/QHash>
#include <QtCore/QString>

namespace boost {

//! Implementation of Boost's hash function for QString
inline std::size_t hash_value(const QString& key)
{
    return qHash(key);
}

} // namespace boost

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0)) // <- ADD_THIS
namespace std {

//! Specialization of C++11 std::hash<> functor for QString
template<> struct hash<QString> {
    inline std::size_t operator()(const QString& key) const
    {
        return qHash(key);
    }
};
#endif // <- ADD_THIS

} // namespace std
