/****************************************************************************
** Copyright (c) 2021, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include <QtCore/QHash>
#include <QtCore/QByteArray>
#include <QtCore/QString>

namespace boost {

// Implementation of Boost's hash function for QByteArray
inline std::size_t hash_value(const QByteArray& key) {
    return qHash(key);
}

// Implementation of Boost's hash function for QString
inline std::size_t hash_value(const QString& key) {
    return qHash(key);
}

} // namespace boost

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace std {

// Specialization of C++11 std::hash<> functor for QByteArray
template<> struct hash<QByteArray> {
    inline std::size_t operator()(const QByteArray& key) const {
        return qHash(key);
    }
};

// Specialization of C++11 std::hash<> functor for QString
template<> struct hash<QString> {
    inline std::size_t operator()(const QString& key) const {
        return qHash(key);
    }
};

} // namespace std
#endif
